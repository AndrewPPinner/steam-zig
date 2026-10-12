#include "steamworks_helpers.h"

#include <atomic>
#include <climits>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace
{
enum CallbackDispatchMode
{
	k_CallbackDispatchModeUninitialized = 0,
	k_CallbackDispatchModeAutomatic = 1,
	k_CallbackDispatchModeManual = 2,
};

std::atomic<int> g_callbackDispatchMode{ k_CallbackDispatchModeUninitialized };
SteamErrMsg g_lastInitError = { 0 };
int g_lastInitResult = 0;
SteamErrMsg g_lastGameServerInitError = { 0 };
int g_lastGameServerInitResult = 0;
CallbackMsg_t g_manualDispatchCallback = {};
std::string g_manualDispatchCallbackData;
bool g_manualDispatchHasCallback = false;
std::string g_manualDispatchAPICallResultData;
bool g_manualDispatchAPICallResultFailed = false;
int g_manualDispatchAPICallResultCallbackID = 0;
SteamworksBytes g_userAuthSessionTicketData;
SteamworksBytes g_gameServerAuthSessionTicketData;

std::string SerializeLowBatteryPower( const LowBatteryPower_t &callback );
std::string SerializeSteamShutdown( const SteamShutdown_t &callback );
std::string SerializeSteamServersConnected( const SteamServersConnected_t &callback );
std::string SerializeSteamServerConnectFailure( const SteamServerConnectFailure_t &callback );
std::string SerializeSteamServersDisconnected( const SteamServersDisconnected_t &callback );
std::string SerializeIPCFailure( const IPCFailure_t &callback );
std::string SerializeLicensesUpdated( const LicensesUpdated_t &callback );
std::string SerializeValidateAuthTicketResponse( const ValidateAuthTicketResponse_t &callback );
std::string SerializeMicroTxnAuthorizationResponse( const MicroTxnAuthorizationResponse_t &callback );
std::string SerializeClientGameServerDeny( const ClientGameServerDeny_t &callback );
std::string SerializeEncryptedAppTicketResponse( const EncryptedAppTicketResponse_t &callback );
std::string SerializeGetAuthSessionTicketResponse( const GetAuthSessionTicketResponse_t &callback );
std::string SerializeGameWebCallback( const GameWebCallback_t &callback );
std::string SerializeDlcInstalled( const DlcInstalled_t &callback );
std::string SerializeNewUrlLaunchParameters( const NewUrlLaunchParameters_t &callback );
std::string SerializeFavoritesListChanged( const FavoritesListChanged_t &callback );
std::string SerializeLobbyInvite( const LobbyInvite_t &callback );
std::string SerializeSteamNetConnectionStatusChanged( const SteamNetConnectionStatusChangedCallback_t &callback );
std::string SerializeLobbyEnter( const LobbyEnter_t &callback );
std::string SerializeLobbyDataUpdate( const LobbyDataUpdate_t &callback );
std::string SerializeLobbyChatUpdate( const LobbyChatUpdate_t &callback );
std::string SerializeLobbyChatMsg( const LobbyChatMsg_t &callback );
std::string SerializeLobbyGameCreated( const LobbyGameCreated_t &callback );
std::string SerializeLobbyKicked( const LobbyKicked_t &callback );
std::string SerializeGameServerChangeRequested( const GameServerChangeRequested_t &callback );
std::string SerializeGameOverlayActivated( const GameOverlayActivated_t &callback );
std::string SerializeGameLobbyJoinRequested( const GameLobbyJoinRequested_t &callback );
std::string SerializeAvatarImageLoaded( const AvatarImageLoaded_t &callback );
std::string SerializeFriendRichPresenceUpdate( const FriendRichPresenceUpdate_t &callback );
std::string SerializeGameRichPresenceJoinRequested( const GameRichPresenceJoinRequested_t &callback );
std::string SerializePersonaStateChange( const PersonaStateChange_t &callback );
std::string SerializeP2PSessionRequest( const P2PSessionRequest_t &callback );
std::string SerializeP2PSessionConnectFail( const P2PSessionConnectFail_t &callback );
std::string SerializeSocketStatusCallback( const SocketStatusCallback_t &callback );
std::string SerializeSteamNetAuthenticationStatus( const SteamNetAuthenticationStatus_t &callback );
std::string SerializeSteamRelayNetworkStatus( const SteamRelayNetworkStatus_t &callback );
std::string SerializeSteamNetworkingMessagesSessionRequest( const SteamNetworkingMessagesSessionRequest_t &callback );
std::string SerializeSteamNetworkingMessagesSessionFailed( const SteamNetworkingMessagesSessionFailed_t &callback );
std::string SerializeSteamNetworkingFakeIPResult( const SteamNetworkingFakeIPResult_t &callback );
std::string SerializeLobbyMatchList( const LobbyMatchList_t &callback );
std::string SerializeLobbyCreated( const LobbyCreated_t &callback );

const char * CallbackDispatchModeName( int mode )
{
	switch ( mode )
	{
	case k_CallbackDispatchModeAutomatic:
		return "automatic";
	case k_CallbackDispatchModeManual:
		return "manual";
	default:
		return "uninitialized";
	}
}

bool TrySelectCallbackDispatchMode( CallbackDispatchMode mode )
{
	int expected = k_CallbackDispatchModeUninitialized;
	return g_callbackDispatchMode.compare_exchange_strong( expected, mode )
		|| expected == mode;
}

void SelectCallbackDispatchMode( CallbackDispatchMode mode, const char *functionName )
{
	if ( TrySelectCallbackDispatchMode( mode ) )
	{
		return;
	}

	throw std::logic_error(
		std::string( functionName )
		+ " requires " + CallbackDispatchModeName( mode )
		+ " callback dispatch, but "
		+ CallbackDispatchModeName( g_callbackDispatchMode.load() )
		+ " callback dispatch is already active"
	);
}

void RequireCallbackDispatchMode( CallbackDispatchMode mode, const char *functionName )
{
	const int activeMode = g_callbackDispatchMode.load();
	if ( activeMode == mode )
	{
		return;
	}

	throw std::logic_error(
		std::string( functionName )
		+ " requires " + CallbackDispatchModeName( mode )
		+ " callback dispatch, but callback dispatch is "
		+ CallbackDispatchModeName( activeMode )
	);
}

class LobbyAsyncState
{
public:
	SteamAPICall_t RequestList()
	{
		m_listPending = false;
		m_listComplete = false;
		m_listIOFailure = false;
		m_lobbyCount = 0;

		SelectCallbackDispatchMode( k_CallbackDispatchModeAutomatic, "Steam_Lobby_RequestList" );

		ISteamMatchmaking *matchmaking = SteamMatchmaking();
		if ( !matchmaking )
		{
			m_listComplete = true;
			m_listIOFailure = true;
			return k_uAPICallInvalid;
		}

		SteamAPICall_t call = matchmaking->RequestLobbyList();
		if ( call == k_uAPICallInvalid )
		{
			m_listComplete = true;
			m_listIOFailure = true;
			return call;
		}

		m_listPending = true;
		m_listCallResult.Set( call, this, &LobbyAsyncState::OnLobbyMatchList );
		return call;
	}

	SteamAPICall_t Join( uint64_steamid lobbyID )
	{
		m_joinPending = false;
		m_joinComplete = false;
		m_joinIOFailure = false;
		m_joinedLobbyID = 0;
		m_joinResponse = 0;

		SelectCallbackDispatchMode( k_CallbackDispatchModeAutomatic, "Steam_Lobby_Join" );

		ISteamMatchmaking *matchmaking = SteamMatchmaking();
		if ( !matchmaking )
		{
			m_joinComplete = true;
			m_joinIOFailure = true;
			return k_uAPICallInvalid;
		}

		SteamAPICall_t call = matchmaking->JoinLobby( CSteamID( lobbyID ) );
		if ( call == k_uAPICallInvalid )
		{
			m_joinComplete = true;
			m_joinIOFailure = true;
			return call;
		}

		m_joinPending = true;
		m_joinCallResult.Set( call, this, &LobbyAsyncState::OnLobbyEnter );
		return call;
	}

	SteamAPICall_t Create( int lobbyType, int maxMembers )
	{
		m_createPending = false;
		m_createComplete = false;
		m_createIOFailure = false;
		m_createdLobbyID = 0;
		m_createResult = k_EResultNone;

		SelectCallbackDispatchMode( k_CallbackDispatchModeAutomatic, "Steam_Lobby_Create" );

		ISteamMatchmaking *matchmaking = SteamMatchmaking();
		if ( !matchmaking )
		{
			m_createComplete = true;
			m_createIOFailure = true;
			return k_uAPICallInvalid;
		}

		SteamAPICall_t call = matchmaking->CreateLobby( static_cast<ELobbyType>( lobbyType ), maxMembers );
		if ( call == k_uAPICallInvalid )
		{
			m_createComplete = true;
			m_createIOFailure = true;
			return call;
		}

		m_createPending = true;
		m_createCallResult.Set( call, this, &LobbyAsyncState::OnLobbyCreated );
		return call;
	}

	bool IsListPending() const { return m_listPending; }
	bool IsListComplete() const { return m_listComplete; }
	bool ListHadIOFailure() const { return m_listIOFailure; }
	uint32 GetLobbyCount() const { return m_lobbyCount; }
	bool IsCreatePending() const { return m_createPending; }
	bool IsCreateComplete() const { return m_createComplete; }
	bool CreateHadIOFailure() const { return m_createIOFailure; }
	uint64_steamid GetCreatedLobbyID() const { return m_createdLobbyID; }
	int GetCreateResult() const { return static_cast<int>( m_createResult ); }
	bool IsJoinPending() const { return m_joinPending; }
	bool IsJoinComplete() const { return m_joinComplete; }
	bool JoinHadIOFailure() const { return m_joinIOFailure; }
	uint64_steamid GetJoinedLobbyID() const { return m_joinedLobbyID; }
	uint32 GetJoinResponse() const { return m_joinResponse; }

	void Clear()
	{
		m_listCallResult.Cancel();
		m_joinCallResult.Cancel();
		m_createCallResult.Cancel();
		m_listPending = false;
		m_listComplete = false;
		m_listIOFailure = false;
		m_lobbyCount = 0;
		m_createPending = false;
		m_createComplete = false;
		m_createIOFailure = false;
		m_createdLobbyID = 0;
		m_createResult = k_EResultNone;
		m_joinPending = false;
		m_joinComplete = false;
		m_joinIOFailure = false;
		m_joinedLobbyID = 0;
		m_joinResponse = 0;
	}

private:
	void OnLobbyMatchList( LobbyMatchList_t *result, bool ioFailure )
	{
		m_listPending = false;
		m_listComplete = true;
		m_listIOFailure = ioFailure;
		m_lobbyCount = result ? result->m_nLobbiesMatching : 0;
	}

	void OnLobbyEnter( LobbyEnter_t *result, bool ioFailure )
	{
		m_joinPending = false;
		m_joinComplete = true;
		m_joinIOFailure = ioFailure;
		m_joinedLobbyID = result ? result->m_ulSteamIDLobby : 0;
		m_joinResponse = result ? result->m_EChatRoomEnterResponse : 0;
	}

	void OnLobbyCreated( LobbyCreated_t *result, bool ioFailure )
	{
		m_createPending = false;
		m_createComplete = true;
		m_createIOFailure = ioFailure;
		m_createResult = result ? result->m_eResult : k_EResultFail;
		m_createdLobbyID = result ? result->m_ulSteamIDLobby : 0;
	}

	CCallResult<LobbyAsyncState, LobbyMatchList_t> m_listCallResult;
	CCallResult<LobbyAsyncState, LobbyEnter_t> m_joinCallResult;
	CCallResult<LobbyAsyncState, LobbyCreated_t> m_createCallResult;
	bool m_listPending = false;
	bool m_listComplete = false;
	bool m_listIOFailure = false;
	uint32 m_lobbyCount = 0;
	bool m_createPending = false;
	bool m_createComplete = false;
	bool m_createIOFailure = false;
	uint64_steamid m_createdLobbyID = 0;
	EResult m_createResult = k_EResultNone;
	bool m_joinPending = false;
	bool m_joinComplete = false;
	bool m_joinIOFailure = false;
	uint64_steamid m_joinedLobbyID = 0;
	uint32 m_joinResponse = 0;
};

LobbyAsyncState g_lobbyAsyncState;

class NetworkingStatusChangedQueue
{
public:
	void Register()
	{
		if ( TrySelectCallbackDispatchMode( k_CallbackDispatchModeAutomatic ) )
		{
			m_callback.Register( this, &NetworkingStatusChangedQueue::OnStatusChanged );
		}
	}

	void Clear()
	{
		m_callback.Unregister();
		m_events.clear();
	}

	std::vector<std::string> PopEvents( int maxEvents )
	{
		std::vector<std::string> result;
		if ( maxEvents <= 0 )
		{
			return result;
		}

		const size_t count = std::min( static_cast<size_t>( maxEvents ), m_events.size() );
		result.reserve( count );
		for ( size_t index = 0; index < count; ++index )
		{
			result.push_back( m_events[index] );
		}
		m_events.erase( m_events.begin(), m_events.begin() + static_cast<std::vector<std::string>::difference_type>( count ) );
		return result;
	}

private:
	void OnStatusChanged( SteamNetConnectionStatusChangedCallback_t *callback )
	{
		if ( !callback )
		{
			return;
		}
		m_events.push_back( SerializeSteamNetConnectionStatusChanged( *callback ) );
	}

	CCallbackManual<NetworkingStatusChangedQueue, SteamNetConnectionStatusChangedCallback_t> m_callback;
	std::vector<std::string> m_events;
};

NetworkingStatusChangedQueue g_networkingStatusChangedQueue;

std::string EscapeEventField( const std::string &value )
{
	std::string escaped;
	escaped.reserve( value.size() );
	for ( char ch : value )
	{
		switch ( ch )
		{
		case '\\':
			escaped += "\\\\";
			break;
		case '\t':
			escaped += "\\t";
			break;
		case '\n':
			escaped += "\\n";
			break;
		case '\r':
			escaped += "\\r";
			break;
		default:
			escaped += ch;
			break;
		}
	}
	return escaped;
}

void AppendPayloadField( std::string &payload, const char *key, const std::string &value )
{
	if ( !payload.empty() )
	{
		payload += '\t';
	}
	payload += key;
	payload += '=';
	payload += EscapeEventField( value );
}

void AppendPayloadField( std::string &payload, const char *key, const char *value )
{
	AppendPayloadField( payload, key, value ? std::string( value ) : std::string() );
}

void AppendPayloadField( std::string &payload, const char *key, bool value )
{
	AppendPayloadField( payload, key, value ? "true" : "false" );
}

template <typename T>
void AppendPayloadNumber( std::string &payload, const char *key, T value )
{
	AppendPayloadField( payload, key, std::to_string( value ) );
}

class LobbyChatQueue
{
public:
	void Register()
	{
		if ( TrySelectCallbackDispatchMode( k_CallbackDispatchModeAutomatic ) )
		{
			m_callback.Register( this, &LobbyChatQueue::OnLobbyChatMessage );
		}
	}

	void Clear()
	{
		m_callback.Unregister();
		m_messages.clear();
	}

	std::vector<std::string> PopMessages( int maxMessages )
	{
		std::vector<std::string> result;
		if ( maxMessages <= 0 )
		{
			return result;
		}

		const size_t count = std::min( static_cast<size_t>( maxMessages ), m_messages.size() );
		result.reserve( count );
		for ( size_t index = 0; index < count; ++index )
		{
			result.push_back( m_messages[index] );
		}
		m_messages.erase( m_messages.begin(), m_messages.begin() + static_cast<std::vector<std::string>::difference_type>( count ) );
		return result;
	}

private:
	void OnLobbyChatMessage( LobbyChatMsg_t *callback )
	{
		if ( !callback )
		{
			return;
		}

		ISteamMatchmaking *matchmaking = SteamMatchmaking();
		if ( !matchmaking )
		{
			return;
		}

		CSteamID user;
		EChatEntryType type = k_EChatEntryTypeInvalid;
		char data[4096] = { 0 };
		const int bytes = matchmaking->GetLobbyChatEntry(
			CSteamID( callback->m_ulSteamIDLobby ),
			callback->m_iChatID,
			&user,
			data,
			sizeof( data ) - 1,
			&type
		);
		if ( bytes < 0 )
		{
			return;
		}
		data[std::min( bytes, static_cast<int>( sizeof( data ) - 1 ) )] = '\0';

		std::string message;
		message.reserve( 256 + static_cast<size_t>( bytes ) );
		message += "lobby=" + std::to_string( callback->m_ulSteamIDLobby );
		message += "\tuser=" + std::to_string( user.ConvertToUint64() );
		message += "\ttype=" + std::to_string( static_cast<int>( type ) );
		message += "\tchat_id=" + std::to_string( callback->m_iChatID );
		message += "\tmessage=" + EscapeEventField( std::string( data ) );
		m_messages.push_back( message );
	}

	CCallbackManual<LobbyChatQueue, LobbyChatMsg_t> m_callback;
	std::vector<std::string> m_messages;
};

LobbyChatQueue g_lobbyChatQueue;

std::string SerializeNetworkingIdentity( const SteamNetworkingIdentity &identity )
{
	char identityString[SteamNetworkingIdentity::k_cchMaxString] = { 0 };
	identity.ToString( identityString, sizeof( identityString ) );

	std::string result;
	result.reserve( 192 );
	result += "identity=" + EscapeEventField( identityString );
	result += "\ttype=" + std::to_string( static_cast<int>( identity.m_eType ) );
	result += "\tsteam_id=" + std::to_string( identity.GetSteamID64() );
	result += "\tipv4=" + std::to_string( identity.GetIPv4() );
	result += "\tfake_ip_type=" + std::to_string( static_cast<int>( identity.GetFakeIPType() ) );
	result += "\tis_fake_ip=" + std::to_string( identity.IsFakeIP() ? 1 : 0 );
	result += "\tis_local_host=" + std::to_string( identity.IsLocalHost() ? 1 : 0 );
	return result;
}

std::string SerializeNetworkingIPAddr( const SteamNetworkingIPAddr &address )
{
	char addressString[SteamNetworkingIPAddr::k_cchMaxString] = { 0 };
	address.ToString( addressString, sizeof( addressString ), true );

	std::string result;
	result.reserve( 160 );
	result += "address=" + EscapeEventField( addressString );
	result += "\tipv4=" + std::to_string( address.GetIPv4() );
	result += "\tport=" + std::to_string( address.m_port );
	result += "\tis_ipv4=" + std::to_string( address.IsIPv4() ? 1 : 0 );
	result += "\tis_local_host=" + std::to_string( address.IsLocalHost() ? 1 : 0 );
	result += "\tfake_ip_type=" + std::to_string( static_cast<int>( address.GetFakeIPType() ) );
	result += "\tis_fake_ip=" + std::to_string( address.IsFakeIP() ? 1 : 0 );
	return result;
}

std::string SerializeNetworkingIPAddrFields( const SteamNetworkingIPAddr &address, const char *prefix )
{
	char addressString[SteamNetworkingIPAddr::k_cchMaxString] = { 0 };
	address.ToString( addressString, sizeof( addressString ), true );

	std::string result;
	result.reserve( 192 );
	result += prefix;
	result += "address=" + EscapeEventField( addressString );
	result += "\t";
	result += prefix;
	result += "ipv4=" + std::to_string( address.GetIPv4() );
	result += "\t";
	result += prefix;
	result += "port=" + std::to_string( address.m_port );
	result += "\t";
	result += prefix;
	result += "is_ipv4=" + std::to_string( address.IsIPv4() ? 1 : 0 );
	result += "\t";
	result += prefix;
	result += "is_local_host=" + std::to_string( address.IsLocalHost() ? 1 : 0 );
	result += "\t";
	result += prefix;
	result += "fake_ip_type=" + std::to_string( static_cast<int>( address.GetFakeIPType() ) );
	result += "\t";
	result += prefix;
	result += "is_fake_ip=" + std::to_string( address.IsFakeIP() ? 1 : 0 );
	return result;
}

std::string SerializeConnectionInfo( const SteamNetConnectionInfo_t &info )
{
	std::string result;
	result.reserve( 1024 );
	result += SerializeNetworkingIdentity( info.m_identityRemote );
	result += "\tuser_data=" + std::to_string( info.m_nUserData );
	result += "\tlisten_socket=" + std::to_string( static_cast<uint32>( info.m_hListenSocket ) );
	result += "\t" + SerializeNetworkingIPAddrFields( info.m_addrRemote, "remote_" );
	result += "\tremote_pop=" + std::to_string( info.m_idPOPRemote );
	result += "\trelay_pop=" + std::to_string( info.m_idPOPRelay );
	result += "\tstate=" + std::to_string( static_cast<int>( info.m_eState ) );
	result += "\tend_reason=" + std::to_string( info.m_eEndReason );
	result += "\tend_debug=" + EscapeEventField( info.m_szEndDebug );
	result += "\tdescription=" + EscapeEventField( info.m_szConnectionDescription );
	result += "\tflags=" + std::to_string( info.m_nFlags );
	return result;
}

std::string SerializeLowBatteryPower( const LowBatteryPower_t &callback )
{
	std::string result;
	result.reserve( 64 );
	result += "minutes_battery_left=" + std::to_string( callback.m_nMinutesBatteryLeft );
	return result;
}

std::string SerializeSteamShutdown( const SteamShutdown_t &callback )
{
	std::string result;
	result.reserve( 32 );
	result += "shutdown=" + std::string( "1" );
	return result;
}

std::string SerializeSteamServersConnected( const SteamServersConnected_t &callback )
{
	std::string result;
	result.reserve( 32 );
	result += "connected=" + std::string( "1" );
	return result;
}

std::string SerializeSteamServerConnectFailure( const SteamServerConnectFailure_t &callback )
{
	std::string result;
	result.reserve( 96 );
	result += "result=" + std::to_string( static_cast<int>( callback.m_eResult ) );
	result += "\tstill_retrying=" + std::to_string( callback.m_bStillRetrying ? 1 : 0 );
	return result;
}

std::string SerializeSteamServersDisconnected( const SteamServersDisconnected_t &callback )
{
	std::string result;
	result.reserve( 64 );
	result += "result=" + std::to_string( static_cast<int>( callback.m_eResult ) );
	return result;
}

std::string SerializeIPCFailure( const IPCFailure_t &callback )
{
	std::string result;
	result.reserve( 64 );
	result += "failure_type=" + std::to_string( callback.m_eFailureType );
	return result;
}

std::string SerializeLicensesUpdated( const LicensesUpdated_t &callback )
{
	std::string result;
	result.reserve( 32 );
	result += "updated=" + std::string( "1" );
	return result;
}

std::string SerializeValidateAuthTicketResponse( const ValidateAuthTicketResponse_t &callback )
{
	std::string result;
	result.reserve( 160 );
	result += "steam_id=" + std::to_string( callback.m_SteamID.ConvertToUint64() );
	result += "\tauth_session_response=" + std::to_string( static_cast<int>( callback.m_eAuthSessionResponse ) );
	result += "\towner_steam_id=" + std::to_string( callback.m_OwnerSteamID.ConvertToUint64() );
	return result;
}

std::string SerializeMicroTxnAuthorizationResponse( const MicroTxnAuthorizationResponse_t &callback )
{
	std::string result;
	result.reserve( 128 );
	result += "app_id=" + std::to_string( callback.m_unAppID );
	result += "\torder_id=" + std::to_string( callback.m_ulOrderID );
	result += "\tauthorized=" + std::to_string( callback.m_bAuthorized ? 1 : 0 );
	return result;
}

std::string SerializeClientGameServerDeny( const ClientGameServerDeny_t &callback )
{
	std::string result;
	result.reserve( 160 );
	result += "app_id=" + std::to_string( callback.m_uAppID );
	result += "\tgame_server_ip=" + std::to_string( callback.m_unGameServerIP );
	result += "\tgame_server_port=" + std::to_string( callback.m_usGameServerPort );
	result += "\tsecure=" + std::to_string( callback.m_bSecure ? 1 : 0 );
	result += "\treason=" + std::to_string( callback.m_uReason );
	return result;
}

std::string SerializeEncryptedAppTicketResponse( const EncryptedAppTicketResponse_t &callback )
{
	std::string result;
	result.reserve( 64 );
	result += "result=" + std::to_string( static_cast<int>( callback.m_eResult ) );
	return result;
}

std::string SerializeGetAuthSessionTicketResponse( const GetAuthSessionTicketResponse_t &callback )
{
	std::string result;
	result.reserve( 96 );
	result += "auth_ticket=" + std::to_string( callback.m_hAuthTicket );
	result += "\tresult=" + std::to_string( static_cast<int>( callback.m_eResult ) );
	return result;
}

std::string SerializeGameWebCallback( const GameWebCallback_t &callback )
{
	std::string result;
	result.reserve( 320 );
	result += "url=" + EscapeEventField( callback.m_szURL );
	return result;
}

std::string SerializeDlcInstalled( const DlcInstalled_t &callback )
{
	std::string result;
	result.reserve( 64 );
	result += "app_id=" + std::to_string( callback.m_nAppID );
	return result;
}

std::string SerializeNewUrlLaunchParameters( const NewUrlLaunchParameters_t &callback )
{
	std::string result;
	result.reserve( 32 );
	result += "new_url_launch_parameters=" + std::string( "1" );
	return result;
}

std::string SerializeFavoritesListChanged( const FavoritesListChanged_t &callback )
{
	std::string result;
	result.reserve( 192 );
	result += "ip=" + std::to_string( callback.m_nIP );
	result += "\tquery_port=" + std::to_string( callback.m_nQueryPort );
	result += "\tconnection_port=" + std::to_string( callback.m_nConnPort );
	result += "\tapp_id=" + std::to_string( callback.m_nAppID );
	result += "\tflags=" + std::to_string( callback.m_nFlags );
	result += "\tadd=" + std::to_string( callback.m_bAdd ? 1 : 0 );
	result += "\taccount_id=" + std::to_string( callback.m_unAccountId );
	return result;
}

std::string SerializeLobbyInvite( const LobbyInvite_t &callback )
{
	std::string result;
	result.reserve( 160 );
	result += "user=" + std::to_string( callback.m_ulSteamIDUser );
	result += "\tlobby=" + std::to_string( callback.m_ulSteamIDLobby );
	result += "\tgame_id=" + std::to_string( callback.m_ulGameID );
	return result;
}

std::string SerializeSteamNetConnectionStatusChanged( const SteamNetConnectionStatusChangedCallback_t &callback )
{
	std::string result;
	result.reserve( 1152 );
	result += "connection=" + std::to_string( static_cast<uint32>( callback.m_hConn ) );
	result += "\told_state=" + std::to_string( static_cast<int>( callback.m_eOldState ) );
	result += "\t" + SerializeConnectionInfo( callback.m_info );
	return result;
}

std::string SerializeLobbyEnter( const LobbyEnter_t &callback )
{
	std::string result;
	result.reserve( 160 );
	result += "lobby=" + std::to_string( callback.m_ulSteamIDLobby );
	result += "\tchat_permissions=" + std::to_string( callback.m_rgfChatPermissions );
	result += "\tlocked=" + std::to_string( callback.m_bLocked ? 1 : 0 );
	result += "\tresponse=" + std::to_string( callback.m_EChatRoomEnterResponse );
	return result;
}

std::string SerializeLobbyDataUpdate( const LobbyDataUpdate_t &callback )
{
	std::string result;
	result.reserve( 128 );
	result += "lobby=" + std::to_string( callback.m_ulSteamIDLobby );
	result += "\tmember=" + std::to_string( callback.m_ulSteamIDMember );
	result += "\tsuccess=" + std::to_string( callback.m_bSuccess ? 1 : 0 );
	return result;
}

std::string SerializeLobbyChatUpdate( const LobbyChatUpdate_t &callback )
{
	std::string result;
	result.reserve( 192 );
	result += "lobby=" + std::to_string( callback.m_ulSteamIDLobby );
	result += "\tuser_changed=" + std::to_string( callback.m_ulSteamIDUserChanged );
	result += "\tmaking_change=" + std::to_string( callback.m_ulSteamIDMakingChange );
	result += "\tstate_change=" + std::to_string( callback.m_rgfChatMemberStateChange );
	return result;
}

std::string SerializeLobbyChatMsg( const LobbyChatMsg_t &callback )
{
	std::string result;
	result.reserve( 160 );
	result += "lobby=" + std::to_string( callback.m_ulSteamIDLobby );
	result += "\tuser=" + std::to_string( callback.m_ulSteamIDUser );
	result += "\ttype=" + std::to_string( callback.m_eChatEntryType );
	result += "\tchat_id=" + std::to_string( callback.m_iChatID );
	return result;
}

std::string SerializeLobbyGameCreated( const LobbyGameCreated_t &callback )
{
	std::string result;
	result.reserve( 160 );
	result += "lobby=" + std::to_string( callback.m_ulSteamIDLobby );
	result += "\tgame_server=" + std::to_string( callback.m_ulSteamIDGameServer );
	result += "\tip=" + std::to_string( callback.m_unIP );
	result += "\tport=" + std::to_string( callback.m_usPort );
	return result;
}

std::string SerializeLobbyKicked( const LobbyKicked_t &callback )
{
	std::string result;
	result.reserve( 128 );
	result += "lobby=" + std::to_string( callback.m_ulSteamIDLobby );
	result += "\tadmin=" + std::to_string( callback.m_ulSteamIDAdmin );
	result += "\tkicked_due_to_disconnect=" + std::to_string( callback.m_bKickedDueToDisconnect ? 1 : 0 );
	return result;
}

std::string SerializeGameServerChangeRequested( const GameServerChangeRequested_t &callback )
{
	std::string result;
	result.reserve( 160 );
	result += "server=" + EscapeEventField( callback.m_rgchServer );
	result += "\tpassword=" + EscapeEventField( callback.m_rgchPassword );
	return result;
}

std::string SerializeGameOverlayActivated( const GameOverlayActivated_t &callback )
{
	std::string result;
	result.reserve( 128 );
	result += "active=" + std::to_string( callback.m_bActive ? 1 : 0 );
	result += "\tuser_initiated=" + std::to_string( callback.m_bUserInitiated ? 1 : 0 );
	result += "\tapp_id=" + std::to_string( callback.m_nAppID );
	result += "\toverlay_pid=" + std::to_string( callback.m_dwOverlayPID );
	return result;
}

std::string SerializeGameLobbyJoinRequested( const GameLobbyJoinRequested_t &callback )
{
	std::string result;
	result.reserve( 128 );
	result += "lobby=" + std::to_string( callback.m_steamIDLobby.ConvertToUint64() );
	result += "\tfriend=" + std::to_string( callback.m_steamIDFriend.ConvertToUint64() );
	return result;
}

std::string SerializeAvatarImageLoaded( const AvatarImageLoaded_t &callback )
{
	std::string result;
	result.reserve( 128 );
	result += "steam_id=" + std::to_string( callback.m_steamID.ConvertToUint64() );
	result += "\timage=" + std::to_string( callback.m_iImage );
	result += "\twide=" + std::to_string( callback.m_iWide );
	result += "\ttall=" + std::to_string( callback.m_iTall );
	return result;
}

std::string SerializeFriendRichPresenceUpdate( const FriendRichPresenceUpdate_t &callback )
{
	std::string result;
	result.reserve( 96 );
	result += "friend=" + std::to_string( callback.m_steamIDFriend.ConvertToUint64() );
	result += "\tapp_id=" + std::to_string( callback.m_nAppID );
	return result;
}

std::string SerializeGameRichPresenceJoinRequested( const GameRichPresenceJoinRequested_t &callback )
{
	std::string result;
	result.reserve( 384 );
	result += "friend=" + std::to_string( callback.m_steamIDFriend.ConvertToUint64() );
	result += "\tconnect=" + EscapeEventField( callback.m_rgchConnect );
	return result;
}

std::string SerializePersonaStateChange( const PersonaStateChange_t &callback )
{
	std::string result;
	result.reserve( 96 );
	result += "steam_id=" + std::to_string( callback.m_ulSteamID );
	result += "\tchange_flags=" + std::to_string( callback.m_nChangeFlags );
	return result;
}

std::string SerializeP2PSessionRequest( const P2PSessionRequest_t &callback )
{
	std::string result;
	result.reserve( 64 );
	result += "remote_steam_id=" + std::to_string( callback.m_steamIDRemote.ConvertToUint64() );
	return result;
}

std::string SerializeP2PSessionConnectFail( const P2PSessionConnectFail_t &callback )
{
	std::string result;
	result.reserve( 96 );
	result += "remote_steam_id=" + std::to_string( callback.m_steamIDRemote.ConvertToUint64() );
	result += "\terror=" + std::to_string( callback.m_eP2PSessionError );
	return result;
}

std::string SerializeSocketStatusCallback( const SocketStatusCallback_t &callback )
{
	std::string result;
	result.reserve( 160 );
	result += "socket=" + std::to_string( callback.m_hSocket );
	result += "\tlisten_socket=" + std::to_string( callback.m_hListenSocket );
	result += "\tremote_steam_id=" + std::to_string( callback.m_steamIDRemote.ConvertToUint64() );
	result += "\tstate=" + std::to_string( callback.m_eSNetSocketState );
	return result;
}

std::string SerializeSteamNetAuthenticationStatus( const SteamNetAuthenticationStatus_t &callback )
{
	std::string result;
	result.reserve( 320 );
	result += "availability=" + std::to_string( static_cast<int>( callback.m_eAvail ) );
	result += "\tdebug=" + EscapeEventField( callback.m_debugMsg );
	return result;
}

std::string SerializeSteamRelayNetworkStatus( const SteamRelayNetworkStatus_t &callback )
{
	std::string result;
	result.reserve( 448 );
	result += "availability=" + std::to_string( static_cast<int>( callback.m_eAvail ) );
	result += "\tping_measurement_in_progress=" + std::to_string( callback.m_bPingMeasurementInProgress );
	result += "\tnetwork_config_availability=" + std::to_string( static_cast<int>( callback.m_eAvailNetworkConfig ) );
	result += "\tany_relay_availability=" + std::to_string( static_cast<int>( callback.m_eAvailAnyRelay ) );
	result += "\tdebug=" + EscapeEventField( callback.m_debugMsg );
	return result;
}

std::string SerializeSteamNetworkingMessagesSessionRequest( const SteamNetworkingMessagesSessionRequest_t &callback )
{
	std::string result;
	result.reserve( 192 );
	result += SerializeNetworkingIdentity( callback.m_identityRemote );
	return result;
}

std::string SerializeSteamNetworkingMessagesSessionFailed( const SteamNetworkingMessagesSessionFailed_t &callback )
{
	std::string result;
	result.reserve( 1024 );
	result += SerializeConnectionInfo( callback.m_info );
	return result;
}

std::string SerializeSteamNetworkingFakeIPResult( const SteamNetworkingFakeIPResult_t &callback )
{
	std::string result;
	result.reserve( 512 );
	result += "result=" + std::to_string( static_cast<int>( callback.m_eResult ) );
	result += "\t" + SerializeNetworkingIdentity( callback.m_identity );
	result += "\t" + std::string( "ip=" ) + std::to_string( callback.m_unIP );
	result += "\t" + std::string( "port0=" ) + std::to_string( callback.m_unPorts[0] );
	result += "\t" + std::string( "port1=" ) + std::to_string( callback.m_unPorts[1] );
	result += "\t" + std::string( "port2=" ) + std::to_string( callback.m_unPorts[2] );
	result += "\t" + std::string( "port3=" ) + std::to_string( callback.m_unPorts[3] );
	result += "\t" + std::string( "port4=" ) + std::to_string( callback.m_unPorts[4] );
	result += "\t" + std::string( "port5=" ) + std::to_string( callback.m_unPorts[5] );
	result += "\t" + std::string( "port6=" ) + std::to_string( callback.m_unPorts[6] );
	result += "\t" + std::string( "port7=" ) + std::to_string( callback.m_unPorts[7] );
	return result;
}

std::string SerializeLobbyMatchList( const LobbyMatchList_t &callback )
{
	std::string result;
	result.reserve( 48 );
	result += "lobbies_matching=" + std::to_string( callback.m_nLobbiesMatching );
	return result;
}

std::string SerializeLobbyCreated( const LobbyCreated_t &callback )
{
	std::string result;
	result.reserve( 96 );
	result += "result=" + std::to_string( static_cast<int>( callback.m_eResult ) );
	result += "\tlobby=" + std::to_string( callback.m_ulSteamIDLobby );
	return result;
}

template <typename CallbackType>
bool CopyCallbackPayload( const std::string &payload, CallbackType *callback )
{
	if ( !callback || payload.size() < sizeof( CallbackType ) )
	{
		return false;
	}
	std::memcpy( callback, payload.data(), sizeof( CallbackType ) );
	return true;
}

template <typename CallbackType>
bool CopyCurrentManualDispatchCallback( CallbackType *callback )
{
	return g_manualDispatchHasCallback &&
		g_manualDispatchCallback.m_iCallback == CallbackType::k_iCallback &&
		CopyCallbackPayload( g_manualDispatchCallbackData, callback );
}

std::string SerializeGameServerItem( const gameserveritem_t &server )
{
	std::string result;
	result.reserve( 1024 );
	result += "name=" + EscapeEventField( server.GetName() );
	result += "\tconnection_address=" + EscapeEventField( server.m_NetAdr.GetConnectionAddressString() );
	result += "\tquery_address=" + EscapeEventField( server.m_NetAdr.GetQueryAddressString() );
	result += "\tip=" + std::to_string( server.m_NetAdr.GetIP() );
	result += "\tconnection_port=" + std::to_string( server.m_NetAdr.GetConnectionPort() );
	result += "\tquery_port=" + std::to_string( server.m_NetAdr.GetQueryPort() );
	result += "\tping=" + std::to_string( server.m_nPing );
	result += "\thad_successful_response=" + std::to_string( server.m_bHadSuccessfulResponse ? 1 : 0 );
	result += "\tdo_not_refresh=" + std::to_string( server.m_bDoNotRefresh ? 1 : 0 );
	result += "\tgame_dir=" + EscapeEventField( server.m_szGameDir );
	result += "\tmap=" + EscapeEventField( server.m_szMap );
	result += "\tgame_description=" + EscapeEventField( server.m_szGameDescription );
	result += "\tapp_id=" + std::to_string( server.m_nAppID );
	result += "\tplayers=" + std::to_string( server.m_nPlayers );
	result += "\tmax_players=" + std::to_string( server.m_nMaxPlayers );
	result += "\tbot_players=" + std::to_string( server.m_nBotPlayers );
	result += "\tpassword=" + std::to_string( server.m_bPassword ? 1 : 0 );
	result += "\tsecure=" + std::to_string( server.m_bSecure ? 1 : 0 );
	result += "\ttime_last_played=" + std::to_string( server.m_ulTimeLastPlayed );
	result += "\tserver_version=" + std::to_string( server.m_nServerVersion );
	result += "\tgame_tags=" + EscapeEventField( server.m_szGameTags );
	result += "\tsteam_id=" + std::to_string( server.m_steamID.ConvertToUint64() );
	result += "\tcurrent_friend_count=" + std::to_string( server.m_nCurrentFriendCount );
	result += "\ttotal_friend_count=" + std::to_string( server.m_nTotalFriendCount );
	return result;
}

class MatchmakingPingResponse : public ISteamMatchmakingPingResponse
{
public:
	HServerQuery PingServer( uint32 ip, uint16 port )
	{
		Clear();

		ISteamMatchmakingServers *servers = SteamMatchmakingServers();
		if ( !servers )
		{
			m_complete = true;
			m_failed = true;
			return HSERVERQUERY_INVALID;
		}

		m_query = servers->PingServer( ip, port, this );
		if ( m_query == HSERVERQUERY_INVALID )
		{
			m_complete = true;
			m_failed = true;
			return m_query;
		}

		m_pending = true;
		return m_query;
	}

	void ServerResponded( gameserveritem_t &server ) override
	{
		m_pending = false;
		m_complete = true;
		m_failed = false;
		m_server = SerializeGameServerItem( server );
	}

	void ServerFailedToRespond() override
	{
		m_pending = false;
		m_complete = true;
		m_failed = true;
		m_server.clear();
	}

	bool IsPending() const { return m_pending; }
	bool IsComplete() const { return m_complete; }
	bool Failed() const { return m_complete && m_failed; }
	bool Succeeded() const { return m_complete && !m_failed; }
	std::string GetServer() const { return m_server; }

	void Clear()
	{
		CancelPendingQuery();
		m_query = HSERVERQUERY_INVALID;
		m_pending = false;
		m_complete = false;
		m_failed = false;
		m_server.clear();
	}

private:
	void CancelPendingQuery()
	{
		if ( !m_pending || m_query == HSERVERQUERY_INVALID )
		{
			return;
		}

		ISteamMatchmakingServers *servers = SteamMatchmakingServers();
		if ( servers )
		{
			servers->CancelServerQuery( m_query );
		}
	}

	HServerQuery m_query = HSERVERQUERY_INVALID;
	bool m_pending = false;
	bool m_complete = false;
	bool m_failed = false;
	std::string m_server;
};

MatchmakingPingResponse g_matchmakingPingResponse;

class MatchmakingServerFriendsResponse : public ISteamMatchmakingServerFriendsResponse
{
public:
	HServerQuery ServerFriends( uint32 ip, uint16 port )
	{
		Clear();

		ISteamMatchmakingServers *servers = SteamMatchmakingServers();
		if ( !servers )
		{
			m_complete = true;
			m_failed = true;
			return HSERVERQUERY_INVALID;
		}

		m_query = servers->ServerFriends( ip, port, this );
		if ( m_query == HSERVERQUERY_INVALID )
		{
			m_complete = true;
			m_failed = true;
			return m_query;
		}

		m_pending = true;
		return m_query;
	}

	void AddFriendToList( CSteamID steamID, const char *pchName, bool bCurrentlyConnected ) override
	{
		std::string friendInfo;
		friendInfo.reserve( 256 );
		friendInfo += "steam_id=" + std::to_string( steamID.ConvertToUint64() );
		friendInfo += "\tname=" + EscapeEventField( pchName ? pchName : "" );
		friendInfo += "\tcurrently_connected=" + std::to_string( bCurrentlyConnected ? 1 : 0 );
		m_friends.push_back( friendInfo );
	}

	void FriendsFailedToRespond() override
	{
		m_pending = false;
		m_complete = true;
		m_failed = true;
	}

	void FriendsRefreshComplete() override
	{
		m_pending = false;
		m_complete = true;
		m_failed = false;
	}

	bool IsPending() const { return m_pending; }
	bool IsComplete() const { return m_complete; }
	bool Failed() const { return m_complete && m_failed; }
	bool Succeeded() const { return m_complete && !m_failed; }
	std::vector<std::string> GetFriends() const { return m_friends; }

	void Clear()
	{
		CancelPendingQuery();
		m_query = HSERVERQUERY_INVALID;
		m_pending = false;
		m_complete = false;
		m_failed = false;
		m_friends.clear();
	}

private:
	void CancelPendingQuery()
	{
		if ( !m_pending || m_query == HSERVERQUERY_INVALID )
		{
			return;
		}

		ISteamMatchmakingServers *servers = SteamMatchmakingServers();
		if ( servers )
		{
			servers->CancelServerQuery( m_query );
		}
	}

	HServerQuery m_query = HSERVERQUERY_INVALID;
	bool m_pending = false;
	bool m_complete = false;
	bool m_failed = false;
	std::vector<std::string> m_friends;
};

MatchmakingServerFriendsResponse g_matchmakingServerFriendsResponse;

SteamNetworkingIdentity IdentityForSteamID( uint64_steamid steamID )
{
	SteamNetworkingIdentity identity;
	identity.SetSteamID64( steamID );
	return identity;
}

std::vector<std::string> ReceiveMessagesOnConnectionStrings( ISteamNetworkingSockets *sockets, HSteamNetConnection connection, int maxMessages )
{
	std::vector<std::string> result;
	if ( !sockets || maxMessages <= 0 )
	{
		return result;
	}

	std::vector<SteamNetworkingMessage_t *> messages( static_cast<size_t>( maxMessages ), nullptr );
	const int count = sockets->ReceiveMessagesOnConnection( connection, messages.data(), maxMessages );
	if ( count <= 0 )
	{
		return result;
	}

	result.reserve( static_cast<size_t>( count ) );
	for ( int index = 0; index < count; ++index )
	{
		SteamNetworkingMessage_t *message = messages[static_cast<size_t>( index )];
		if ( message )
		{
			result.emplace_back( static_cast<const char *>( message->m_pData ), static_cast<size_t>( message->m_cbSize ) );
			message->Release();
		}
	}
	return result;
}

std::vector<std::string> ReceiveMessagesOnPollGroupStrings( ISteamNetworkingSockets *sockets, HSteamNetPollGroup pollGroup, int maxMessages )
{
	std::vector<std::string> result;
	if ( !sockets || maxMessages <= 0 )
	{
		return result;
	}

	std::vector<SteamNetworkingMessage_t *> messages( static_cast<size_t>( maxMessages ), nullptr );
	const int count = sockets->ReceiveMessagesOnPollGroup( pollGroup, messages.data(), maxMessages );
	if ( count <= 0 )
	{
		return result;
	}

	result.reserve( static_cast<size_t>( count ) );
	for ( int index = 0; index < count; ++index )
	{
		SteamNetworkingMessage_t *message = messages[static_cast<size_t>( index )];
		if ( message )
		{
			result.emplace_back( static_cast<const char *>( message->m_pData ), static_cast<size_t>( message->m_cbSize ) );
			message->Release();
		}
	}
	return result;
}

SteamworksBytesVector ReceiveMessagesOnChannelBytes( ISteamNetworkingMessages *messagesInterface, int localChannel, int maxMessages )
{
	SteamworksBytesVector result;
	if ( !messagesInterface || maxMessages <= 0 )
	{
		return result;
	}

	std::vector<SteamNetworkingMessage_t *> messages( static_cast<size_t>( maxMessages ), nullptr );
	const int count = messagesInterface->ReceiveMessagesOnChannel( localChannel, messages.data(), maxMessages );
	if ( count <= 0 )
	{
		return result;
	}

	result.reserve( static_cast<size_t>( count ) );
	for ( int index = 0; index < count; ++index )
	{
		SteamNetworkingMessage_t *message = messages[static_cast<size_t>( index )];
		if ( message )
		{
			result.emplace_back( static_cast<const char *>( message->m_pData ), static_cast<size_t>( message->m_cbSize ) );
			message->Release();
		}
	}
	return result;
}

bool SetHTTPRequestRawPostBodyBytes( ISteamHTTP *http, HTTPRequestHandle request, const char *contentType, const uint8_t *data, size_t dataSize )
{
	if ( !http || !contentType || ( !data && dataSize > 0 ) || dataSize > UINT32_MAX )
	{
		return false;
	}
	return http->SetHTTPRequestRawPostBody(
		request,
		contentType,
		const_cast<uint8 *>( reinterpret_cast<const uint8 *>( data ) ),
		static_cast<uint32>( dataSize )
	);
}

SteamworksBytes GetHTTPResponseBodyDataBytes( ISteamHTTP *http, HTTPRequestHandle request, uint32 dataSize )
{
	if ( !http || dataSize == 0 )
	{
		return {};
	}

	SteamworksBytes data( static_cast<size_t>( dataSize ), '\0' );
	if ( !http->GetHTTPResponseBodyData( request, reinterpret_cast<uint8 *>( data.data() ), dataSize ) )
	{
		return {};
	}
	return data;
}

SteamworksBytes GetHTTPStreamingResponseBodyDataBytes( ISteamHTTP *http, HTTPRequestHandle request, uint32 offset, uint32 dataSize )
{
	if ( !http || dataSize == 0 )
	{
		return {};
	}

	SteamworksBytes data( static_cast<size_t>( dataSize ), '\0' );
	if ( !http->GetHTTPStreamingResponseBodyData( request, offset, reinterpret_cast<uint8 *>( data.data() ), dataSize ) )
	{
		return {};
	}
	return data;
}

bool GetFriendGameInfo( uint64_steamid friendID, FriendGameInfo_t *info )
{
	ISteamFriends *friends = SteamFriends();
	if ( !friends || !info )
	{
		return false;
	}
	return friends->GetFriendGamePlayed( CSteamID( friendID ), info );
}
}

bool Steam_Init()
{
	return SteamAPI_Init();
}

int Steam_InitEx()
{
	g_lastInitError[0] = '\0';
	g_lastInitResult = static_cast<int>( SteamAPI_InitEx( &g_lastInitError ) );
	return g_lastInitResult;
}

int Steam_InitFlat()
{
	g_lastInitError[0] = '\0';
	// SteamAPI_InitFlat was added after SteamAPI_InitEx. Use the common
	// implementation so this wrapper remains available with older SDKs.
	g_lastInitResult = static_cast<int>( SteamAPI_InitEx( &g_lastInitError ) );
	return g_lastInitResult;
}

void Steam_ClearHelperState()
{
	g_lobbyAsyncState.Clear();
	g_networkingStatusChangedQueue.Clear();
	g_lobbyChatQueue.Clear();
	g_matchmakingPingResponse.Clear();
	g_matchmakingServerFriendsResponse.Clear();
}

void Steam_Shutdown()
{
	Steam_NetworkingSockets_RunCallbacks();
	SteamAPI_RunCallbacks();
	Steam_NetworkingSockets_RunCallbacks();
	Steam_ClearHelperState();
	SteamAPI_Shutdown();
	g_callbackDispatchMode.store( k_CallbackDispatchModeUninitialized );
}

void Steam_ShutdownManualDispatch()
{
	const HSteamPipe pipe = SteamAPI_GetHSteamPipe();
	SteamAPI_ManualDispatch_RunFrame( pipe );
	while ( SteamAPI_ManualDispatch_GetNextCallback( pipe, &g_manualDispatchCallback ) )
	{
		SteamAPI_ManualDispatch_FreeLastCallback( pipe );
	}
	Steam_ClearHelperState();
	SteamAPI_Shutdown();
	g_callbackDispatchMode.store( k_CallbackDispatchModeUninitialized );
}

void Steam_RunCallbacks()
{
	SelectCallbackDispatchMode( k_CallbackDispatchModeAutomatic, "Steam_RunCallbacks" );
	SteamAPI_RunCallbacks();
}

int Steam_GetCallbackDispatchMode()
{
	return g_callbackDispatchMode.load();
}

int Steam_CallbackDispatchModeUninitialized()
{
	return k_CallbackDispatchModeUninitialized;
}

int Steam_CallbackDispatchModeAutomatic()
{
	return k_CallbackDispatchModeAutomatic;
}

int Steam_CallbackDispatchModeManual()
{
	return k_CallbackDispatchModeManual;
}

bool Steam_IsSteamRunning()
{
	return SteamAPI_IsSteamRunning();
}

bool Steam_RestartAppIfNecessary( AppId_t appID )
{
	return SteamAPI_RestartAppIfNecessary( appID );
}

void Steam_ReleaseCurrentThreadMemory()
{
	SteamAPI_ReleaseCurrentThreadMemory();
}

void Steam_WriteMiniDump( uint32 structuredExceptionCode, uint32 buildID )
{
	SteamAPI_WriteMiniDump( structuredExceptionCode, nullptr, buildID );
}

const char * Steam_GetSteamInstallPath()
{
	return SteamAPI_GetSteamInstallPath();
}

void Steam_SetTryCatchCallbacks( bool enabled )
{
	SteamAPI_SetTryCatchCallbacks( enabled );
}

void Steam_SetMiniDumpComment( const char * message )
{
	SteamAPI_SetMiniDumpComment( message );
}

void Steam_ManualDispatch_Init()
{
	SelectCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_Init" );
	SteamAPI_ManualDispatch_Init();
}

void Steam_ManualDispatch_RunFrame( HSteamPipe pipe )
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_RunFrame" );
	SteamAPI_ManualDispatch_RunFrame( pipe );
}

bool Steam_ManualDispatch_GetNextCallback( HSteamPipe pipe )
{
	g_manualDispatchCallback = {};
	g_manualDispatchCallbackData.clear();
	g_manualDispatchHasCallback = false;

	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_GetNextCallback" );

	CallbackMsg_t callback = {};
	if ( !SteamAPI_ManualDispatch_GetNextCallback( pipe, &callback ) )
	{
		return false;
	}

	g_manualDispatchCallback = callback;
	if ( callback.m_pubParam && callback.m_cubParam > 0 )
	{
		g_manualDispatchCallbackData.assign(
			reinterpret_cast<const char *>( callback.m_pubParam ),
			static_cast<size_t>( callback.m_cubParam )
		);
	}
	g_manualDispatchHasCallback = true;
	return true;
}

HSteamUser Steam_ManualDispatch_GetCallbackSteamUser()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_GetCallbackSteamUser" );
	return g_manualDispatchHasCallback ? g_manualDispatchCallback.m_hSteamUser : 0;
}

int Steam_ManualDispatch_GetCallbackID()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_GetCallbackID" );
	return g_manualDispatchHasCallback ? g_manualDispatchCallback.m_iCallback : 0;
}

std::string Steam_ManualDispatch_GetCallbackData()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_GetCallbackData" );
	return g_manualDispatchCallbackData;
}

int Steam_ManualDispatch_GetCallbackSize()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_GetCallbackSize" );
	return g_manualDispatchHasCallback ? g_manualDispatchCallback.m_cubParam : 0;
}

bool Steam_ManualDispatch_CallbackIsAPICallCompleted()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_CallbackIsAPICallCompleted" );
	return g_manualDispatchHasCallback && g_manualDispatchCallback.m_iCallback == SteamAPICallCompleted_t::k_iCallback;
}

SteamAPICall_t Steam_ManualDispatch_GetCompletedAPICall()
{
	if ( !Steam_ManualDispatch_CallbackIsAPICallCompleted() ||
		g_manualDispatchCallbackData.size() < sizeof( SteamAPICallCompleted_t ) )
	{
		return k_uAPICallInvalid;
	}

	SteamAPICallCompleted_t completed = {};
	std::memcpy( &completed, g_manualDispatchCallbackData.data(), sizeof( completed ) );
	return completed.m_hAsyncCall;
}

int Steam_ManualDispatch_GetCompletedCallbackID()
{
	if ( !Steam_ManualDispatch_CallbackIsAPICallCompleted() ||
		g_manualDispatchCallbackData.size() < sizeof( SteamAPICallCompleted_t ) )
	{
		return 0;
	}

	SteamAPICallCompleted_t completed = {};
	std::memcpy( &completed, g_manualDispatchCallbackData.data(), sizeof( completed ) );
	return completed.m_iCallback;
}

uint32 Steam_ManualDispatch_GetCompletedCallbackSize()
{
	if ( !Steam_ManualDispatch_CallbackIsAPICallCompleted() ||
		g_manualDispatchCallbackData.size() < sizeof( SteamAPICallCompleted_t ) )
	{
		return 0;
	}

	SteamAPICallCompleted_t completed = {};
	std::memcpy( &completed, g_manualDispatchCallbackData.data(), sizeof( completed ) );
	return completed.m_cubParam;
}

bool Steam_ManualDispatch_GetAPICallResult( HSteamPipe pipe, SteamAPICall_t apiCall, int callbackSize, int callbackID )
{
	g_manualDispatchAPICallResultData.clear();
	g_manualDispatchAPICallResultFailed = false;
	g_manualDispatchAPICallResultCallbackID = 0;
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_GetAPICallResult" );
	if ( callbackSize <= 0 )
	{
		return false;
	}

	std::string resultData;
	resultData.resize( static_cast<size_t>( callbackSize ) );
	bool failed = false;
	const bool ok = SteamAPI_ManualDispatch_GetAPICallResult(
		pipe,
		apiCall,
		resultData.data(),
		callbackSize,
		callbackID,
		&failed
	);
	if ( !ok )
	{
		return false;
	}

	g_manualDispatchAPICallResultData = resultData;
	g_manualDispatchAPICallResultFailed = failed;
	g_manualDispatchAPICallResultCallbackID = callbackID;
	return true;
}

std::string Steam_ManualDispatch_GetAPICallResultData()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_GetAPICallResultData" );
	return g_manualDispatchAPICallResultData;
}

bool Steam_ManualDispatch_GetAPICallResultFailed()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_GetAPICallResultFailed" );
	return g_manualDispatchAPICallResultFailed;
}

int Steam_ManualDispatch_CallbackIDSteamAPICallCompleted()
{
	return SteamAPICallCompleted_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLowBatteryPower()
{
	return LowBatteryPower_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamShutdown()
{
	return SteamShutdown_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamServersConnected()
{
	return SteamServersConnected_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamServerConnectFailure()
{
	return SteamServerConnectFailure_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamServersDisconnected()
{
	return SteamServersDisconnected_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDIPCFailure()
{
	return IPCFailure_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLicensesUpdated()
{
	return LicensesUpdated_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDValidateAuthTicketResponse()
{
	return ValidateAuthTicketResponse_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDMicroTxnAuthorizationResponse()
{
	return MicroTxnAuthorizationResponse_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDClientGameServerDeny()
{
	return ClientGameServerDeny_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDEncryptedAppTicketResponse()
{
	return EncryptedAppTicketResponse_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDGetAuthSessionTicketResponse()
{
	return GetAuthSessionTicketResponse_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDGameWebCallback()
{
	return GameWebCallback_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDDlcInstalled()
{
	return DlcInstalled_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDNewUrlLaunchParameters()
{
	return NewUrlLaunchParameters_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDFavoritesListChanged()
{
	return FavoritesListChanged_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyInvite()
{
	return LobbyInvite_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamNetConnectionStatusChanged()
{
	return SteamNetConnectionStatusChangedCallback_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyEnter()
{
	return LobbyEnter_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyDataUpdate()
{
	return LobbyDataUpdate_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyChatUpdate()
{
	return LobbyChatUpdate_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyChatMsg()
{
	return LobbyChatMsg_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyGameCreated()
{
	return LobbyGameCreated_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyKicked()
{
	return LobbyKicked_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDGameServerChangeRequested()
{
	return GameServerChangeRequested_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDGameOverlayActivated()
{
	return GameOverlayActivated_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDGameLobbyJoinRequested()
{
	return GameLobbyJoinRequested_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDAvatarImageLoaded()
{
	return AvatarImageLoaded_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDFriendRichPresenceUpdate()
{
	return FriendRichPresenceUpdate_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDGameRichPresenceJoinRequested()
{
	return GameRichPresenceJoinRequested_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDPersonaStateChange()
{
	return PersonaStateChange_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDP2PSessionRequest()
{
	return P2PSessionRequest_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDP2PSessionConnectFail()
{
	return P2PSessionConnectFail_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSocketStatusCallback()
{
	return SocketStatusCallback_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamNetAuthenticationStatus()
{
	return SteamNetAuthenticationStatus_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamRelayNetworkStatus()
{
	return SteamRelayNetworkStatus_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamNetworkingMessagesSessionRequest()
{
	return SteamNetworkingMessagesSessionRequest_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamNetworkingMessagesSessionFailed()
{
	return SteamNetworkingMessagesSessionFailed_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDSteamNetworkingFakeIPResult()
{
	return SteamNetworkingFakeIPResult_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyMatchList()
{
	return LobbyMatchList_t::k_iCallback;
}

int Steam_ManualDispatch_CallbackIDLobbyCreated()
{
	return LobbyCreated_t::k_iCallback;
}

std::string Steam_ManualDispatch_DecodeCallbackLowBatteryPower()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLowBatteryPower" );
	LowBatteryPower_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLowBatteryPower( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamShutdown()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamShutdown" );
	SteamShutdown_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamShutdown( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamServersConnected()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamServersConnected" );
	SteamServersConnected_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamServersConnected( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamServerConnectFailure()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamServerConnectFailure" );
	SteamServerConnectFailure_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamServerConnectFailure( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamServersDisconnected()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamServersDisconnected" );
	SteamServersDisconnected_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamServersDisconnected( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackIPCFailure()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackIPCFailure" );
	IPCFailure_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeIPCFailure( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackLicensesUpdated()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLicensesUpdated" );
	LicensesUpdated_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLicensesUpdated( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackValidateAuthTicketResponse()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackValidateAuthTicketResponse" );
	ValidateAuthTicketResponse_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeValidateAuthTicketResponse( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackMicroTxnAuthorizationResponse()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackMicroTxnAuthorizationResponse" );
	MicroTxnAuthorizationResponse_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeMicroTxnAuthorizationResponse( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackClientGameServerDeny()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackClientGameServerDeny" );
	ClientGameServerDeny_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeClientGameServerDeny( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackEncryptedAppTicketResponse()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackEncryptedAppTicketResponse" );
	EncryptedAppTicketResponse_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeEncryptedAppTicketResponse( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackGetAuthSessionTicketResponse()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackGetAuthSessionTicketResponse" );
	GetAuthSessionTicketResponse_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeGetAuthSessionTicketResponse( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackGameWebCallback()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackGameWebCallback" );
	GameWebCallback_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeGameWebCallback( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackDlcInstalled()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackDlcInstalled" );
	DlcInstalled_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeDlcInstalled( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackNewUrlLaunchParameters()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackNewUrlLaunchParameters" );
	NewUrlLaunchParameters_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeNewUrlLaunchParameters( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackFavoritesListChanged()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackFavoritesListChanged" );
	FavoritesListChanged_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeFavoritesListChanged( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackLobbyInvite()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLobbyInvite" );
	LobbyInvite_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLobbyInvite( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamNetConnectionStatusChanged()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamNetConnectionStatusChanged" );
	SteamNetConnectionStatusChangedCallback_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamNetConnectionStatusChanged( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackLobbyEnter()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLobbyEnter" );
	LobbyEnter_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLobbyEnter( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackLobbyDataUpdate()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLobbyDataUpdate" );
	LobbyDataUpdate_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLobbyDataUpdate( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackLobbyChatUpdate()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLobbyChatUpdate" );
	LobbyChatUpdate_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLobbyChatUpdate( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackLobbyChatMsg()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLobbyChatMsg" );
	LobbyChatMsg_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLobbyChatMsg( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackLobbyGameCreated()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLobbyGameCreated" );
	LobbyGameCreated_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLobbyGameCreated( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackLobbyKicked()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackLobbyKicked" );
	LobbyKicked_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeLobbyKicked( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackGameServerChangeRequested()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackGameServerChangeRequested" );
	GameServerChangeRequested_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeGameServerChangeRequested( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackGameOverlayActivated()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackGameOverlayActivated" );
	GameOverlayActivated_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeGameOverlayActivated( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackGameLobbyJoinRequested()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackGameLobbyJoinRequested" );
	GameLobbyJoinRequested_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeGameLobbyJoinRequested( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackAvatarImageLoaded()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackAvatarImageLoaded" );
	AvatarImageLoaded_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeAvatarImageLoaded( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackFriendRichPresenceUpdate()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackFriendRichPresenceUpdate" );
	FriendRichPresenceUpdate_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeFriendRichPresenceUpdate( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackGameRichPresenceJoinRequested()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackGameRichPresenceJoinRequested" );
	GameRichPresenceJoinRequested_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeGameRichPresenceJoinRequested( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackPersonaStateChange()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackPersonaStateChange" );
	PersonaStateChange_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializePersonaStateChange( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackP2PSessionRequest()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackP2PSessionRequest" );
	P2PSessionRequest_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeP2PSessionRequest( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackP2PSessionConnectFail()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackP2PSessionConnectFail" );
	P2PSessionConnectFail_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeP2PSessionConnectFail( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSocketStatusCallback()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSocketStatusCallback" );
	SocketStatusCallback_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSocketStatusCallback( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamNetAuthenticationStatus()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamNetAuthenticationStatus" );
	SteamNetAuthenticationStatus_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamNetAuthenticationStatus( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamRelayNetworkStatus()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamRelayNetworkStatus" );
	SteamRelayNetworkStatus_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamRelayNetworkStatus( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamNetworkingMessagesSessionRequest()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamNetworkingMessagesSessionRequest" );
	SteamNetworkingMessagesSessionRequest_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamNetworkingMessagesSessionRequest( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamNetworkingMessagesSessionFailed()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamNetworkingMessagesSessionFailed" );
	SteamNetworkingMessagesSessionFailed_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamNetworkingMessagesSessionFailed( callback );
}

std::string Steam_ManualDispatch_DecodeCallbackSteamNetworkingFakeIPResult()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeCallbackSteamNetworkingFakeIPResult" );
	SteamNetworkingFakeIPResult_t callback = {};
	if ( !CopyCurrentManualDispatchCallback( &callback ) )
	{
		return {};
	}
	return SerializeSteamNetworkingFakeIPResult( callback );
}

std::string Steam_ManualDispatch_DecodeAPICallResultLobbyMatchList()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeAPICallResultLobbyMatchList" );
	if ( g_manualDispatchAPICallResultCallbackID != LobbyMatchList_t::k_iCallback )
	{
		return {};
	}

	LobbyMatchList_t callback = {};
	if ( !CopyCallbackPayload( g_manualDispatchAPICallResultData, &callback ) )
	{
		return {};
	}
	return SerializeLobbyMatchList( callback );
}

std::string Steam_ManualDispatch_DecodeAPICallResultLobbyEnter()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeAPICallResultLobbyEnter" );
	if ( g_manualDispatchAPICallResultCallbackID != LobbyEnter_t::k_iCallback )
	{
		return {};
	}

	LobbyEnter_t callback = {};
	if ( !CopyCallbackPayload( g_manualDispatchAPICallResultData, &callback ) )
	{
		return {};
	}
	return SerializeLobbyEnter( callback );
}

std::string Steam_ManualDispatch_DecodeAPICallResultLobbyCreated()
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_DecodeAPICallResultLobbyCreated" );
	if ( g_manualDispatchAPICallResultCallbackID != LobbyCreated_t::k_iCallback )
	{
		return {};
	}

	LobbyCreated_t callback = {};
	if ( !CopyCallbackPayload( g_manualDispatchAPICallResultData, &callback ) )
	{
		return {};
	}
	return SerializeLobbyCreated( callback );
}

void Steam_ManualDispatch_FreeLastCallback( HSteamPipe pipe )
{
	RequireCallbackDispatchMode( k_CallbackDispatchModeManual, "Steam_ManualDispatch_FreeLastCallback" );
	if ( g_manualDispatchHasCallback )
	{
		SteamAPI_ManualDispatch_FreeLastCallback( pipe );
	}
	g_manualDispatchCallback = {};
	g_manualDispatchCallbackData.clear();
	g_manualDispatchHasCallback = false;
}

HSteamPipe Steam_GetHSteamPipe()
{
	return SteamAPI_GetHSteamPipe();
}

HSteamUser Steam_GetHSteamUser()
{
	return SteamAPI_GetHSteamUser();
}

int Steam_GetLastInitResult()
{
	return g_lastInitResult;
}

const char * Steam_GetLastInitError()
{
	return g_lastInitError;
}

std::string Steam_Apps_GetDLCDataByIndexString( int dlcIndex )
{
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return {};
	}

	AppId_t appID = k_uAppIdInvalid;
	bool available = false;
	char name[256] = { 0 };
	if ( !apps->BGetDLCDataByIndex( dlcIndex, &appID, &available, name, sizeof( name ) ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadNumber( payload, "app_id", appID );
	AppendPayloadField( payload, "available", available );
	AppendPayloadField( payload, "name", name );
	return payload;
}

std::string Steam_Apps_GetTimedTrialString()
{
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return {};
	}

	uint32 secondsAllowed = 0;
	uint32 secondsPlayed = 0;
	if ( !apps->BIsTimedTrial( &secondsAllowed, &secondsPlayed ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadNumber( payload, "seconds_allowed", secondsAllowed );
	AppendPayloadNumber( payload, "seconds_played", secondsPlayed );
	return payload;
}

std::string Steam_Apps_GetAppInstallDirString( AppId_t appID )
{
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return {};
	}

	std::vector<char> buffer( 4096, '\0' );
	const uint32 written = apps->GetAppInstallDir( appID, buffer.data(), static_cast<uint32>( buffer.size() ) );
	if ( written == 0 )
	{
		return {};
	}
	return std::string( buffer.data() );
}

std::string Steam_Apps_GetBetaInfoString( int betaIndex )
{
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return {};
	}

	uint32 flags = 0;
	uint32 buildID = 0;
	uint32 lastUpdated = 0;
	char betaName[256] = { 0 };
	char description[1024] = { 0 };
	if ( !apps->GetBetaInfo( betaIndex, &flags, &buildID, betaName, sizeof( betaName ), description, sizeof( description ), &lastUpdated ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadNumber( payload, "flags", flags );
	AppendPayloadNumber( payload, "build_id", buildID );
	AppendPayloadField( payload, "name", betaName );
	AppendPayloadField( payload, "description", description );
	AppendPayloadNumber( payload, "last_updated", lastUpdated );
	return payload;
}

std::string Steam_Apps_GetCurrentBetaNameString()
{
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return {};
	}

	char name[256] = { 0 };
	return apps->GetCurrentBetaName( name, sizeof( name ) ) ? std::string( name ) : std::string();
}

std::string Steam_Apps_GetDlcDownloadProgressString( AppId_t appID )
{
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return {};
	}

	uint64 bytesDownloaded = 0;
	uint64 bytesTotal = 0;
	if ( !apps->GetDlcDownloadProgress( appID, &bytesDownloaded, &bytesTotal ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadNumber( payload, "bytes_downloaded", bytesDownloaded );
	AppendPayloadNumber( payload, "bytes_total", bytesTotal );
	return payload;
}

std::vector<std::string> Steam_Apps_GetInstalledDepotsList( AppId_t appID )
{
	std::vector<std::string> result;
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return result;
	}

	std::vector<DepotId_t> depots( 256 );
	const uint32 count = apps->GetInstalledDepots( appID, depots.data(), static_cast<uint32>( depots.size() ) );
	result.reserve( count );
	for ( uint32 index = 0; index < count && index < depots.size(); ++index )
	{
		result.emplace_back( std::to_string( depots[index] ) );
	}
	return result;
}

std::string Steam_Apps_GetLaunchCommandLineString()
{
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return {};
	}

	std::vector<char> buffer( 4096, '\0' );
	const int written = apps->GetLaunchCommandLine( buffer.data(), static_cast<int>( buffer.size() ) );
	if ( written <= 0 )
	{
		return {};
	}
	return std::string( buffer.data() );
}

std::string Steam_Apps_GetNumBetasString()
{
	ISteamApps *apps = SteamApps();
	if ( !apps )
	{
		return {};
	}

	int available = 0;
	int privateCount = 0;
	const int total = apps->GetNumBetas( &available, &privateCount );
	std::string payload;
	AppendPayloadNumber( payload, "total", total );
	AppendPayloadNumber( payload, "available", available );
	AppendPayloadNumber( payload, "private", privateCount );
	return payload;
}

std::vector<std::string> Steam_Inventory_GetItemDefinitionIDsList()
{
	std::vector<std::string> result;
	ISteamInventory *inventory = SteamInventory();
	if ( !inventory )
	{
		return result;
	}

	uint32 count = 0;
	if ( !inventory->GetItemDefinitionIDs( nullptr, &count ) || count == 0 )
	{
		return result;
	}

	std::vector<SteamItemDef_t> definitions( count );
	if ( !inventory->GetItemDefinitionIDs( definitions.data(), &count ) )
	{
		return result;
	}

	result.reserve( count );
	for ( uint32 index = 0; index < count && index < definitions.size(); ++index )
	{
		result.emplace_back( std::to_string( definitions[index] ) );
	}
	return result;
}

std::string Steam_Inventory_GetItemDefinitionPropertyString( SteamItemDef_t definition, const char *propertyName )
{
	ISteamInventory *inventory = SteamInventory();
	if ( !inventory )
	{
		return {};
	}

	uint32 valueSize = 0;
	if ( !inventory->GetItemDefinitionProperty( definition, propertyName, nullptr, &valueSize ) || valueSize == 0 )
	{
		return {};
	}

	std::vector<char> buffer( valueSize, '\0' );
	return inventory->GetItemDefinitionProperty( definition, propertyName, buffer.data(), &valueSize ) ? std::string( buffer.data() ) : std::string();
}

std::string Steam_Inventory_GetItemPriceString( SteamItemDef_t definition )
{
	ISteamInventory *inventory = SteamInventory();
	if ( !inventory )
	{
		return {};
	}

	uint64 currentPrice = 0;
	uint64 basePrice = 0;
	if ( !inventory->GetItemPrice( definition, &currentPrice, &basePrice ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadNumber( payload, "current_price", currentPrice );
	AppendPayloadNumber( payload, "base_price", basePrice );
	return payload;
}

std::vector<std::string> Steam_Inventory_GetItemsWithPricesList()
{
	std::vector<std::string> result;
	ISteamInventory *inventory = SteamInventory();
	if ( !inventory )
	{
		return result;
	}

	const uint32 count = inventory->GetNumItemsWithPrices();
	if ( count == 0 )
	{
		return result;
	}

	std::vector<SteamItemDef_t> definitions( count );
	std::vector<uint64> currentPrices( count );
	std::vector<uint64> basePrices( count );
	if ( !inventory->GetItemsWithPrices( definitions.data(), currentPrices.data(), basePrices.data(), count ) )
	{
		return result;
	}

	result.reserve( count );
	for ( uint32 index = 0; index < count; ++index )
	{
		std::string payload;
		AppendPayloadNumber( payload, "item_def", definitions[index] );
		AppendPayloadNumber( payload, "current_price", currentPrices[index] );
		AppendPayloadNumber( payload, "base_price", basePrices[index] );
		result.push_back( payload );
	}
	return result;
}

std::string Steam_Inventory_GetResultItemPropertyString( SteamInventoryResult_t resultHandle, uint32 itemIndex, const char *propertyName )
{
	ISteamInventory *inventory = SteamInventory();
	if ( !inventory )
	{
		return {};
	}

	uint32 valueSize = 0;
	if ( !inventory->GetResultItemProperty( resultHandle, itemIndex, propertyName, nullptr, &valueSize ) || valueSize == 0 )
	{
		return {};
	}

	std::vector<char> buffer( valueSize, '\0' );
	return inventory->GetResultItemProperty( resultHandle, itemIndex, propertyName, buffer.data(), &valueSize ) ? std::string( buffer.data() ) : std::string();
}

std::string Steam_UGC_GetItemDownloadInfoString( PublishedFileId_t publishedFileID )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	uint64 bytesDownloaded = 0;
	uint64 bytesTotal = 0;
	if ( !ugc->GetItemDownloadInfo( publishedFileID, &bytesDownloaded, &bytesTotal ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadNumber( payload, "bytes_downloaded", bytesDownloaded );
	AppendPayloadNumber( payload, "bytes_total", bytesTotal );
	return payload;
}

std::string Steam_UGC_GetItemInstallInfoString( PublishedFileId_t publishedFileID )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	uint64 sizeOnDisk = 0;
	uint32 timestamp = 0;
	std::vector<char> folder( 4096, '\0' );
	if ( !ugc->GetItemInstallInfo( publishedFileID, &sizeOnDisk, folder.data(), static_cast<uint32>( folder.size() ), &timestamp ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadNumber( payload, "size_on_disk", sizeOnDisk );
	AppendPayloadField( payload, "folder", folder.data() );
	AppendPayloadNumber( payload, "timestamp", timestamp );
	return payload;
}

std::string Steam_UGC_GetItemUpdateProgressString( UGCUpdateHandle_t updateHandle )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	uint64 bytesProcessed = 0;
	uint64 bytesTotal = 0;
	const EItemUpdateStatus status = ugc->GetItemUpdateProgress( updateHandle, &bytesProcessed, &bytesTotal );
	std::string payload;
	AppendPayloadNumber( payload, "status", static_cast<int>( status ) );
	AppendPayloadNumber( payload, "bytes_processed", bytesProcessed );
	AppendPayloadNumber( payload, "bytes_total", bytesTotal );
	return payload;
}

std::string Steam_UGC_GetQueryUGCAdditionalPreviewString( UGCQueryHandle_t queryHandle, uint32 index, uint32 previewIndex )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	char urlOrVideoID[4096] = { 0 };
	char originalFileName[1024] = { 0 };
	EItemPreviewType previewType = k_EItemPreviewType_Image;
	if ( !ugc->GetQueryUGCAdditionalPreview( queryHandle, index, previewIndex, urlOrVideoID, sizeof( urlOrVideoID ), originalFileName, sizeof( originalFileName ), &previewType ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadField( payload, "url_or_video_id", urlOrVideoID );
	AppendPayloadField( payload, "original_file_name", originalFileName );
	AppendPayloadNumber( payload, "preview_type", static_cast<int>( previewType ) );
	return payload;
}

std::string Steam_UGC_GetQueryUGCKeyValueTagByIndexString( UGCQueryHandle_t queryHandle, uint32 index, uint32 tagIndex )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	char key[1024] = { 0 };
	char value[4096] = { 0 };
	if ( !ugc->GetQueryUGCKeyValueTag( queryHandle, index, tagIndex, key, sizeof( key ), value, sizeof( value ) ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadField( payload, "key", key );
	AppendPayloadField( payload, "value", value );
	return payload;
}

std::string Steam_UGC_GetQueryUGCKeyValueTagValueString( UGCQueryHandle_t queryHandle, uint32 index, const char *key )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	char value[4096] = { 0 };
	return ugc->GetQueryUGCKeyValueTag( queryHandle, index, key, value, sizeof( value ) ) ? std::string( value ) : std::string();
}

std::string Steam_UGC_GetQueryUGCMetadataString( UGCQueryHandle_t queryHandle, uint32 index )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	char metadata[4096] = { 0 };
	return ugc->GetQueryUGCMetadata( queryHandle, index, metadata, sizeof( metadata ) ) ? std::string( metadata ) : std::string();
}

std::string Steam_UGC_GetQueryUGCPreviewURLString( UGCQueryHandle_t queryHandle, uint32 index )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	char url[4096] = { 0 };
	return ugc->GetQueryUGCPreviewURL( queryHandle, index, url, sizeof( url ) ) ? std::string( url ) : std::string();
}

std::string Steam_UGC_GetQueryUGCTagDisplayNameString( UGCQueryHandle_t queryHandle, uint32 index, uint32 tagIndex )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	char value[1024] = { 0 };
	return ugc->GetQueryUGCTagDisplayName( queryHandle, index, tagIndex, value, sizeof( value ) ) ? std::string( value ) : std::string();
}

std::string Steam_UGC_GetQueryUGCTagString( UGCQueryHandle_t queryHandle, uint32 index, uint32 tagIndex )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	char value[1024] = { 0 };
	return ugc->GetQueryUGCTag( queryHandle, index, tagIndex, value, sizeof( value ) ) ? std::string( value ) : std::string();
}

std::string Steam_UGC_GetSupportedGameVersionDataString( UGCQueryHandle_t queryHandle, uint32 index, uint32 versionIndex )
{
	ISteamUGC *ugc = SteamUGC();
	if ( !ugc )
	{
		return {};
	}

	char minBranch[256] = { 0 };
	char maxBranch[256] = { 0 };
	if ( !ugc->GetSupportedGameVersionData( queryHandle, index, versionIndex, minBranch, maxBranch, sizeof( minBranch ) ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadField( payload, "game_branch_min", minBranch );
	AppendPayloadField( payload, "game_branch_max", maxBranch );
	return payload;
}

std::string Steam_User_GetAvailableVoiceString( uint32 sampleRate )
{
	ISteamUser *user = SteamUser();
	if ( !user )
	{
		return {};
	}

	uint32 compressed = 0;
	uint32 uncompressed = 0;
	const EVoiceResult result = user->GetAvailableVoice( &compressed, &uncompressed, sampleRate );
	std::string payload;
	AppendPayloadNumber( payload, "result", static_cast<int>( result ) );
	AppendPayloadNumber( payload, "compressed", compressed );
	AppendPayloadNumber( payload, "uncompressed", uncompressed );
	return payload;
}

std::string Steam_User_GetUserDataFolderString()
{
	ISteamUser *user = SteamUser();
	if ( !user )
	{
		return {};
	}

	std::vector<char> buffer( 4096, '\0' );
	return user->GetUserDataFolder( buffer.data(), static_cast<int>( buffer.size() ) ) ? std::string( buffer.data() ) : std::string();
}

std::string Steam_Utils_FilterTextString( int context, uint64_steamid sourceSteamID, const char *inputMessage )
{
	ISteamUtils *utils = SteamUtils();
	if ( !utils || !inputMessage )
	{
		return {};
	}

	const size_t inputSize = std::strlen( inputMessage );
	std::vector<char> buffer( std::max<size_t>( 256, inputSize * 4 + 1 ), '\0' );
	const int written = utils->FilterText(
		static_cast<ETextFilteringContext>( context ),
		CSteamID( sourceSteamID ),
		inputMessage,
		buffer.data(),
		static_cast<uint32>( buffer.size() )
	);
	return written >= 0 ? std::string( buffer.data() ) : std::string();
}

std::string Steam_Utils_GetEnteredGamepadTextInputString()
{
	ISteamUtils *utils = SteamUtils();
	if ( !utils )
	{
		return {};
	}

	std::vector<char> buffer( 4096, '\0' );
	return utils->GetEnteredGamepadTextInput( buffer.data(), static_cast<uint32>( buffer.size() ) ) ? std::string( buffer.data() ) : std::string();
}

std::string Steam_Utils_GetImageSizeString( int image )
{
	ISteamUtils *utils = SteamUtils();
	if ( !utils )
	{
		return {};
	}

	uint32 width = 0;
	uint32 height = 0;
	if ( !utils->GetImageSize( image, &width, &height ) )
	{
		return {};
	}

	std::string payload;
	AppendPayloadNumber( payload, "width", width );
	AppendPayloadNumber( payload, "height", height );
	return payload;
}

std::string Steam_Video_IsBroadcastingString()
{
	ISteamVideo *video = SteamVideo();
	if ( !video )
	{
		return {};
	}

	int viewers = 0;
	const bool broadcasting = video->IsBroadcasting( &viewers );
	std::string payload;
	AppendPayloadField( payload, "broadcasting", broadcasting );
	AppendPayloadNumber( payload, "viewers", viewers );
	return payload;
}

std::string Steam_Apps_IsTimedTrialString()
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}

	uint32 punSecondsAllowed = {};
	uint32 punSecondsPlayed = {};
	const auto result = SteamAPI_ISteamApps_BIsTimedTrial( self, &punSecondsAllowed, &punSecondsPlayed );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "seconds_allowed", punSecondsAllowed );
	AppendPayloadNumber( payload, "seconds_played", punSecondsPlayed );
	return payload;
}

std::string Steam_Controller_GetControllerBindingRevisionString( ControllerHandle_t controllerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}

	int pMajor = {};
	int pMinor = {};
	const auto result = SteamAPI_ISteamController_GetControllerBindingRevision( self, controllerHandle, &pMajor, &pMinor );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "major", pMajor );
	AppendPayloadNumber( payload, "minor", pMinor );
	return payload;
}

std::string Steam_Friends_GetClanActivityCountsString( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}

	int pnOnline = {};
	int pnInGame = {};
	int pnChatting = {};
	const auto result = SteamAPI_ISteamFriends_GetClanActivityCounts( self, steamIDClan, &pnOnline, &pnInGame, &pnChatting );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "online", pnOnline );
	AppendPayloadNumber( payload, "in_game", pnInGame );
	AppendPayloadNumber( payload, "chatting", pnChatting );
	return payload;
}

std::string Steam_HTTP_GetHTTPDownloadProgressPctString( HTTPRequestHandle hRequest )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}

	float pflPercentOut = {};
	const auto result = SteamAPI_ISteamHTTP_GetHTTPDownloadProgressPct( self, hRequest, &pflPercentOut );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "percent", pflPercentOut );
	return payload;
}

std::string Steam_HTTP_GetHTTPRequestWasTimedOutString( HTTPRequestHandle hRequest )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}

	bool pbWasTimedOut = {};
	const auto result = SteamAPI_ISteamHTTP_GetHTTPRequestWasTimedOut( self, hRequest, &pbWasTimedOut );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadField( payload, "was_timed", pbWasTimedOut );
	return payload;
}

std::string Steam_HTTP_GetHTTPResponseBodySizeString( HTTPRequestHandle hRequest )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}

	uint32 unBodySize = {};
	const auto result = SteamAPI_ISteamHTTP_GetHTTPResponseBodySize( self, hRequest, &unBodySize );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "body_size", unBodySize );
	return payload;
}

std::string Steam_Input_GetDeviceBindingRevisionString( InputHandle_t inputHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}

	int pMajor = {};
	int pMinor = {};
	const auto result = SteamAPI_ISteamInput_GetDeviceBindingRevision( self, inputHandle, &pMajor, &pMinor );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "major", pMajor );
	AppendPayloadNumber( payload, "minor", pMinor );
	return payload;
}

std::string Steam_Matchmaking_GetFavoriteGameString( int iGame )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}

	AppId_t pnAppID = {};
	uint32 pnIP = {};
	uint16 pnConnPort = {};
	uint16 pnQueryPort = {};
	uint32 punFlags = {};
	uint32 pRTime32LastPlayedOnServer = {};
	const auto result = SteamAPI_ISteamMatchmaking_GetFavoriteGame( self, iGame, &pnAppID, &pnIP, &pnConnPort, &pnQueryPort, &punFlags, &pRTime32LastPlayedOnServer );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "app_id", pnAppID );
	AppendPayloadNumber( payload, "ip", pnIP );
	AppendPayloadNumber( payload, "conn_port", pnConnPort );
	AppendPayloadNumber( payload, "query_port", pnQueryPort );
	AppendPayloadNumber( payload, "flags", punFlags );
	AppendPayloadNumber( payload, "r_time_32_last_played_on_server", pRTime32LastPlayedOnServer );
	return payload;
}

std::string Steam_Matchmaking_GetLobbyDataByIndexString( uint64_steamid steamIDLobby, int iLobbyData )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}

	char pchKey[4096] = { 0 };
	char pchValue[4096] = { 0 };
	const auto result = SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex( self, steamIDLobby, iLobbyData, pchKey, sizeof( pchKey ), pchValue, sizeof( pchValue ) );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadField( payload, "key", pchKey );
	AppendPayloadField( payload, "value", pchValue );
	return payload;
}

std::string Steam_Networking_IsDataAvailableOnSocketString( SNetSocket_t hSocket )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}

	uint32 pcubMsgSize = {};
	const auto result = SteamAPI_ISteamNetworking_IsDataAvailableOnSocket( self, hSocket, &pcubMsgSize );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "msg_size", pcubMsgSize );
	return payload;
}

std::string Steam_Networking_IsDataAvailableString( SNetListenSocket_t hListenSocket )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}

	uint32 pcubMsgSize = {};
	SNetSocket_t phSocket = {};
	const auto result = SteamAPI_ISteamNetworking_IsDataAvailable( self, hListenSocket, &pcubMsgSize, &phSocket );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "msg_size", pcubMsgSize );
	AppendPayloadNumber( payload, "h_socket", phSocket );
	return payload;
}

std::string Steam_Networking_IsP2PPacketAvailableString( int nChannel )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}

	uint32 pcubMsgSize = {};
	const auto result = SteamAPI_ISteamNetworking_IsP2PPacketAvailable( self, &pcubMsgSize, nChannel );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "msg_size", pcubMsgSize );
	return payload;
}

std::string Steam_Parties_GetNumAvailableBeaconLocationsString()
{
	auto *self = SteamAPI_SteamParties_v002();
	if ( !self )
	{
		return {};
	}

	uint32 puNumLocations = {};
	const auto result = SteamAPI_ISteamParties_GetNumAvailableBeaconLocations( self, &puNumLocations );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "num_locations", puNumLocations );
	return payload;
}

std::string Steam_RemotePlay_GetSessionClientResolutionString( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}

	int pnResolutionX = {};
	int pnResolutionY = {};
	const auto result = SteamAPI_ISteamRemotePlay_BGetSessionClientResolution( self, unSessionID, &pnResolutionX, &pnResolutionY );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "resolution_x", pnResolutionX );
	AppendPayloadNumber( payload, "resolution_y", pnResolutionY );
	return payload;
}

std::string Steam_RemoteStorage_GetQuotaString()
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}

	uint64 pnTotalBytes = {};
	uint64 puAvailableBytes = {};
	const auto result = SteamAPI_ISteamRemoteStorage_GetQuota( self, &pnTotalBytes, &puAvailableBytes );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "total_bytes", pnTotalBytes );
	AppendPayloadNumber( payload, "available_bytes", puAvailableBytes );
	return payload;
}

std::string Steam_RemoteStorage_GetUGCDownloadProgressString( UGCHandle_t hContent )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}

	int32 pnBytesDownloaded = {};
	int32 pnBytesExpected = {};
	const auto result = SteamAPI_ISteamRemoteStorage_GetUGCDownloadProgress( self, hContent, &pnBytesDownloaded, &pnBytesExpected );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "bytes_downloaded", pnBytesDownloaded );
	AppendPayloadNumber( payload, "bytes_expected", pnBytesExpected );
	return payload;
}

std::string Steam_UGC_GetQueryUGCKeyValueTagString( UGCQueryHandle_t handle, uint32 index, uint32 keyValueTagIndex )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}

	char pchKey[4096] = { 0 };
	char pchValue[4096] = { 0 };
	const auto result = SteamAPI_ISteamUGC_GetQueryUGCKeyValueTag( self, handle, index, keyValueTagIndex, pchKey, sizeof( pchKey ), pchValue, sizeof( pchValue ) );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadField( payload, "key", pchKey );
	AppendPayloadField( payload, "value", pchValue );
	return payload;
}

std::string Steam_UGC_GetQueryUGCStatisticString( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}

	uint64 pStatValue = {};
	const auto result = SteamAPI_ISteamUGC_GetQueryUGCStatistic( self, handle, index, eStatType, &pStatValue );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadNumber( payload, "stat_value", pStatValue );
	return payload;
}

std::string Steam_UserStats_GetMostAchievedAchievementInfoString()
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}

	char pchName[4096] = { 0 };
	float pflPercent = {};
	bool pbAchieved = {};
	const auto result = SteamAPI_ISteamUserStats_GetMostAchievedAchievementInfo( self, pchName, sizeof( pchName ), &pflPercent, &pbAchieved );
	std::string payload;
	AppendPayloadNumber( payload, "result", result );
	AppendPayloadField( payload, "name", pchName );
	AppendPayloadNumber( payload, "percent", pflPercent );
	AppendPayloadField( payload, "achieved", pbAchieved );
	return payload;
}

std::string Steam_UserStats_GetNextMostAchievedAchievementInfoString( int iIteratorPrevious )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}

	char pchName[4096] = { 0 };
	float pflPercent = {};
	bool pbAchieved = {};
	const auto result = SteamAPI_ISteamUserStats_GetNextMostAchievedAchievementInfo( self, iIteratorPrevious, pchName, sizeof( pchName ), &pflPercent, &pbAchieved );
	std::string payload;
	AppendPayloadNumber( payload, "result", result );
	AppendPayloadField( payload, "name", pchName );
	AppendPayloadNumber( payload, "percent", pflPercent );
	AppendPayloadField( payload, "achieved", pbAchieved );
	return payload;
}

std::string Steam_Utils_IsAPICallCompletedString( SteamAPICall_t hSteamAPICall )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}

	bool pbFailed = {};
	const auto result = SteamAPI_ISteamUtils_IsAPICallCompleted( self, hSteamAPICall, &pbFailed );
	const bool ok = result;
	if ( !ok )
	{
		return {};
	}
	std::string payload;
	AppendPayloadField( payload, "failed", pbFailed );
	return payload;
}

bool Steam_GameServer_Init( uint32 ip, uint16 gamePort, uint16 queryPort, int serverMode, const char * versionString )
{
	return SteamGameServer_Init( ip, gamePort, queryPort, static_cast<EServerMode>( serverMode ), versionString );
}

int Steam_GameServer_InitEx( uint32 ip, uint16 gamePort, uint16 queryPort, int serverMode, const char * versionString )
{
	g_lastGameServerInitError[0] = '\0';
	g_lastGameServerInitResult = static_cast<int>(
		SteamGameServer_InitEx( ip, gamePort, queryPort, static_cast<EServerMode>( serverMode ), versionString, &g_lastGameServerInitError )
	);
	return g_lastGameServerInitResult;
}

void Steam_GameServer_Shutdown()
{
	SteamGameServer_Shutdown();
	g_callbackDispatchMode.store( k_CallbackDispatchModeUninitialized );
}

void Steam_GameServer_RunCallbacks()
{
	SelectCallbackDispatchMode( k_CallbackDispatchModeAutomatic, "Steam_GameServer_RunCallbacks" );
	SteamGameServer_RunCallbacks();
}

void Steam_GameServer_ReleaseCurrentThreadMemory()
{
	SteamGameServer_ReleaseCurrentThreadMemory();
}

bool Steam_GameServer_GlobalBSecure()
{
	return SteamGameServer_BSecure();
}

uint64 Steam_GameServer_GlobalGetSteamID()
{
	return SteamGameServer_GetSteamID();
}

HSteamPipe Steam_GameServer_GetHSteamPipe()
{
	return SteamGameServer_GetHSteamPipe();
}

HSteamUser Steam_GameServer_GetHSteamUser()
{
	return SteamGameServer_GetHSteamUser();
}

int Steam_GameServer_GetLastInitResult()
{
	return g_lastGameServerInitResult;
}

const char * Steam_GameServer_GetLastInitError()
{
	return g_lastGameServerInitError;
}

int Steam_ServerModeInvalid()
{
	return static_cast<int>( eServerModeInvalid );
}

int Steam_ServerModeNoAuthentication()
{
	return static_cast<int>( eServerModeNoAuthentication );
}

int Steam_ServerModeAuthentication()
{
	return static_cast<int>( eServerModeAuthentication );
}

int Steam_ServerModeAuthenticationAndSecure()
{
	return static_cast<int>( eServerModeAuthenticationAndSecure );
}

uint16 Steam_GameServer_QueryPortShared()
{
	return STEAMGAMESERVER_QUERY_PORT_SHARED;
}

HAuthTicket Steam_User_GetAuthSessionTicketBytes( int maxTicketSize )
{
	g_userAuthSessionTicketData.clear();
	auto *user = SteamAPI_SteamUser();
	if ( !user || maxTicketSize <= 0 )
	{
		return k_HAuthTicketInvalid;
	}

	SteamworksBytes ticket( static_cast<size_t>( maxTicketSize ), '\0' );
	uint32 ticketSize = 0;
	const HAuthTicket handle = user->GetAuthSessionTicket( ticket.data(), maxTicketSize, &ticketSize, nullptr );
	if ( ticketSize > ticket.size() )
	{
		return handle;
	}

	ticket.resize( static_cast<size_t>( ticketSize ) );
	g_userAuthSessionTicketData = ticket;
	return handle;
}

SteamworksBytes Steam_User_GetLastAuthSessionTicketBytes()
{
	return g_userAuthSessionTicketData;
}

int Steam_User_BeginAuthSessionBytes( const uint8_t * data, size_t dataSize, uint64_steamid steamID )
{
	auto *user = SteamAPI_SteamUser();
	if ( !user || ( !data && dataSize > 0 ) || dataSize > static_cast<size_t>( INT32_MAX ) )
	{
		return k_EBeginAuthSessionResultInvalidTicket;
	}
	return static_cast<int>( user->BeginAuthSession( data, static_cast<int>( dataSize ), CSteamID( steamID ) ) );
}

SteamAPICall_t Steam_User_RequestEncryptedAppTicketBytes( const uint8_t * data, size_t dataSize )
{
	auto *user = SteamAPI_SteamUser();
	if ( !user || ( !data && dataSize > 0 ) || dataSize > static_cast<size_t>( INT32_MAX ) )
	{
		return k_uAPICallInvalid;
	}
	return user->RequestEncryptedAppTicket( const_cast<uint8_t *>( data ), static_cast<int>( dataSize ) );
}

SteamworksBytes Steam_User_GetEncryptedAppTicketBytes( int maxTicketSize )
{
	auto *user = SteamAPI_SteamUser();
	if ( !user || maxTicketSize <= 0 )
	{
		return {};
	}

	SteamworksBytes ticket( static_cast<size_t>( maxTicketSize ), '\0' );
	uint32 ticketSize = 0;
	if ( !user->GetEncryptedAppTicket( ticket.data(), maxTicketSize, &ticketSize ) || ticketSize > ticket.size() )
	{
		return {};
	}

	ticket.resize( static_cast<size_t>( ticketSize ) );
	return ticket;
}

HAuthTicket Steam_GameServer_GetAuthSessionTicketBytes( int maxTicketSize )
{
	g_gameServerAuthSessionTicketData.clear();
	auto *server = SteamAPI_SteamGameServer();
	if ( !server || maxTicketSize <= 0 )
	{
		return k_HAuthTicketInvalid;
	}

	SteamworksBytes ticket( static_cast<size_t>( maxTicketSize ), '\0' );
	uint32 ticketSize = 0;
	const HAuthTicket handle = server->GetAuthSessionTicket( ticket.data(), maxTicketSize, &ticketSize, nullptr );
	if ( ticketSize > ticket.size() )
	{
		return handle;
	}

	ticket.resize( static_cast<size_t>( ticketSize ) );
	g_gameServerAuthSessionTicketData = ticket;
	return handle;
}

SteamworksBytes Steam_GameServer_GetLastAuthSessionTicketBytes()
{
	return g_gameServerAuthSessionTicketData;
}

int Steam_GameServer_BeginAuthSessionBytes( const uint8_t * data, size_t dataSize, uint64_steamid steamID )
{
	auto *server = SteamAPI_SteamGameServer();
	if ( !server || ( !data && dataSize > 0 ) || dataSize > static_cast<size_t>( INT32_MAX ) )
	{
		return k_EBeginAuthSessionResultInvalidTicket;
	}
	return static_cast<int>( server->BeginAuthSession( data, static_cast<int>( dataSize ), CSteamID( steamID ) ) );
}

HSteamListenSocket Steam_NetworkingSockets_CreateListenSocketIPStringNoOptions( const char * address )
{
	g_networkingStatusChangedQueue.Register();
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets || !address )
	{
		return HSteamListenSocket{};
	}

	SteamNetworkingIPAddr parsedAddress;
	if ( !parsedAddress.ParseString( address ) )
	{
		return HSteamListenSocket{};
	}

	return sockets->CreateListenSocketIP( parsedAddress, 0, nullptr );
}

HSteamNetConnection Steam_NetworkingSockets_ConnectByIPAddressStringNoOptions( const char * address )
{
	g_networkingStatusChangedQueue.Register();
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets || !address )
	{
		return HSteamNetConnection{};
	}

	SteamNetworkingIPAddr parsedAddress;
	if ( !parsedAddress.ParseString( address ) )
	{
		return HSteamNetConnection{};
	}

	return sockets->ConnectByIPAddress( parsedAddress, 0, nullptr );
}

HSteamListenSocket Steam_NetworkingSockets_CreateListenSocketP2PNoOptions( int localVirtualPort )
{
	g_networkingStatusChangedQueue.Register();
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	return sockets ? sockets->CreateListenSocketP2P( localVirtualPort, 0, nullptr ) : HSteamListenSocket{};
}

HSteamNetConnection Steam_NetworkingSockets_ConnectP2PSteamIDNoOptions( uint64_steamid steamID, int remoteVirtualPort )
{
	g_networkingStatusChangedQueue.Register();
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return HSteamNetConnection{};
	}

	SteamNetworkingIdentity identity;
	identity.SetSteamID64( steamID );
	return sockets->ConnectP2P( identity, remoteVirtualPort, 0, nullptr );
}

void Steam_NetworkingSockets_EnableConnectionStatusCallbacks()
{
	SelectCallbackDispatchMode(
		k_CallbackDispatchModeAutomatic,
		"Steam_NetworkingSockets_EnableConnectionStatusCallbacks"
	);
	g_networkingStatusChangedQueue.Register();
}

void Steam_NetworkingSockets_ClearConnectionStatusChangedEvents()
{
	g_networkingStatusChangedQueue.Clear();
}

std::vector<std::string> Steam_NetworkingSockets_PollConnectionStatusChangedStrings( int maxEvents )
{
	return g_networkingStatusChangedQueue.PopEvents( maxEvents );
}

int Steam_NetworkingSockets_SendMessageToConnectionString( HSteamNetConnection connection, const char * data, int sendFlags )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets || !data )
	{
		return k_EResultInvalidState;
	}
	const size_t dataSize = std::strlen( data );
	if ( dataSize > UINT32_MAX )
	{
		return k_EResultInvalidState;
	}
	return static_cast<int>( sockets->SendMessageToConnection( connection, data, static_cast<uint32>( dataSize ), sendFlags, nullptr ) );
}

int Steam_NetworkingSockets_SendMessageToConnectionBytes( HSteamNetConnection connection, const uint8_t * data, size_t dataSize, int sendFlags )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets || ( !data && dataSize > 0 ) || dataSize > UINT32_MAX )
	{
		return k_EResultInvalidState;
	}
	return static_cast<int>( sockets->SendMessageToConnection( connection, data, static_cast<uint32>( dataSize ), sendFlags, nullptr ) );
}

std::vector<std::string> Steam_NetworkingSockets_ReceiveMessagesOnConnectionStrings( HSteamNetConnection connection, int maxMessages )
{
	return ReceiveMessagesOnConnectionStrings( SteamAPI_SteamNetworkingSockets_SteamAPI(), connection, maxMessages );
}

SteamworksBytesVector Steam_NetworkingSockets_ReceiveMessagesOnConnectionBytes( HSteamNetConnection connection, int maxMessages )
{
	return ReceiveMessagesOnConnectionStrings( SteamAPI_SteamNetworkingSockets_SteamAPI(), connection, maxMessages );
}

std::vector<std::string> Steam_NetworkingSockets_ReceiveMessagesOnPollGroupStrings( HSteamNetPollGroup pollGroup, int maxMessages )
{
	return ReceiveMessagesOnPollGroupStrings( SteamAPI_SteamNetworkingSockets_SteamAPI(), pollGroup, maxMessages );
}

SteamworksBytesVector Steam_NetworkingSockets_ReceiveMessagesOnPollGroupBytes( HSteamNetPollGroup pollGroup, int maxMessages )
{
	return ReceiveMessagesOnPollGroupStrings( SteamAPI_SteamNetworkingSockets_SteamAPI(), pollGroup, maxMessages );
}

std::string Steam_NetworkingSockets_GetConnectionNameString( HSteamNetConnection connection )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	char buffer[256] = { 0 };
	return sockets->GetConnectionName( connection, buffer, sizeof( buffer ) ) ? std::string( buffer ) : std::string();
}

std::string Steam_NetworkingSockets_GetConnectionInfoString( HSteamNetConnection connection )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	SteamNetConnectionInfo_t info = {};
	return sockets->GetConnectionInfo( connection, &info ) ? SerializeConnectionInfo( info ) : std::string();
}

std::string Steam_NetworkingSockets_GetConnectionRealTimeStatusString( HSteamNetConnection connection )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	SteamNetConnectionRealTimeStatus_t status = {};
	const EResult result = sockets->GetConnectionRealTimeStatus( connection, &status, 0, nullptr );

	std::string serialized;
	serialized.reserve( 512 );
	serialized += "result=" + std::to_string( static_cast<int>( result ) );
	serialized += "\tstate=" + std::to_string( static_cast<int>( status.m_eState ) );
	serialized += "\tping=" + std::to_string( status.m_nPing );
	serialized += "\tconnection_quality_local=" + std::to_string( status.m_flConnectionQualityLocal );
	serialized += "\tconnection_quality_remote=" + std::to_string( status.m_flConnectionQualityRemote );
	serialized += "\tout_packets_per_sec=" + std::to_string( status.m_flOutPacketsPerSec );
	serialized += "\tout_bytes_per_sec=" + std::to_string( status.m_flOutBytesPerSec );
	serialized += "\tin_packets_per_sec=" + std::to_string( status.m_flInPacketsPerSec );
	serialized += "\tin_bytes_per_sec=" + std::to_string( status.m_flInBytesPerSec );
	serialized += "\tsend_rate_bytes_per_sec=" + std::to_string( status.m_nSendRateBytesPerSecond );
	serialized += "\tpending_unreliable=" + std::to_string( status.m_cbPendingUnreliable );
	serialized += "\tpending_reliable=" + std::to_string( status.m_cbPendingReliable );
	serialized += "\tsent_unacked_reliable=" + std::to_string( status.m_cbSentUnackedReliable );
	serialized += "\tqueue_time_usec=" + std::to_string( status.m_usecQueueTime );
	serialized += "\tmax_jitter_usec=" + std::to_string( status.m_usecMaxJitter );
	return serialized;
}

std::string Steam_NetworkingSockets_GetDetailedConnectionStatusString( HSteamNetConnection connection )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	char buffer[4096] = { 0 };
	const int result = sockets->GetDetailedConnectionStatus( connection, buffer, sizeof( buffer ) );
	return result >= 0 ? std::string( buffer ) : std::string();
}

std::string Steam_NetworkingSockets_GetListenSocketAddressString( HSteamListenSocket listenSocket )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	SteamNetworkingIPAddr address = {};
	return sockets->GetListenSocketAddress( listenSocket, &address ) ? SerializeNetworkingIPAddr( address ) : std::string();
}

std::string Steam_NetworkingSockets_GetIdentityString()
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	SteamNetworkingIdentity identity;
	identity.Clear();
	return sockets->GetIdentity( &identity ) ? SerializeNetworkingIdentity( identity ) : std::string();
}

std::string Steam_NetworkingSockets_GetAuthenticationStatusString()
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	SteamNetAuthenticationStatus_t status = {};
	const ESteamNetworkingAvailability availability = sockets->GetAuthenticationStatus( &status );

	std::string serialized;
	serialized.reserve( 384 );
	serialized += "availability=" + std::to_string( static_cast<int>( availability ) );
	serialized += "\tstatus_availability=" + std::to_string( static_cast<int>( status.m_eAvail ) );
	serialized += "\tdebug=" + EscapeEventField( status.m_debugMsg );
	return serialized;
}

std::string Steam_NetworkingSockets_GetFakeIPString( int firstPortIndex )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	SteamNetworkingFakeIPResult_t info = {};
	sockets->GetFakeIP( firstPortIndex, &info );

	std::string serialized;
	serialized.reserve( 384 );
	serialized += "result=" + std::to_string( static_cast<int>( info.m_eResult ) );
	serialized += "\t" + SerializeNetworkingIdentity( info.m_identity );
	serialized += "\tip=" + std::to_string( info.m_unIP );
	for ( int index = 0; index < SteamNetworkingFakeIPResult_t::k_nMaxReturnPorts; ++index )
	{
		serialized += "\tport";
		serialized += std::to_string( index );
		serialized += "=";
		serialized += std::to_string( info.m_unPorts[index] );
	}
	return serialized;
}

HSteamListenSocket Steam_NetworkingSockets_CreateListenSocketP2PFakeIPNoOptions( int fakePortIndex )
{
	g_networkingStatusChangedQueue.Register();
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	return sockets ? sockets->CreateListenSocketP2PFakeIP( fakePortIndex, 0, nullptr ) : HSteamListenSocket{};
}

std::string Steam_NetworkingSockets_GetRemoteFakeIPForConnectionString( HSteamNetConnection connection )
{
	auto *sockets = SteamAPI_SteamNetworkingSockets_SteamAPI();
	if ( !sockets )
	{
		return {};
	}

	SteamNetworkingIPAddr address = {};
	const EResult result = sockets->GetRemoteFakeIPForConnection( connection, &address );
	std::string serialized;
	serialized.reserve( 256 );
	serialized += "result=" + std::to_string( static_cast<int>( result ) );
	serialized += "\t" + SerializeNetworkingIPAddr( address );
	return serialized;
}

HSteamListenSocket Steam_GameServerNetworkingSockets_CreateListenSocketP2PNoOptions( int localVirtualPort )
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	return sockets ? sockets->CreateListenSocketP2P( localVirtualPort, 0, nullptr ) : HSteamListenSocket{};
}

HSteamNetPollGroup Steam_GameServerNetworkingSockets_CreatePollGroup()
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	return sockets ? sockets->CreatePollGroup() : HSteamNetPollGroup{};
}

bool Steam_GameServerNetworkingSockets_DestroyPollGroup( HSteamNetPollGroup pollGroup )
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	return sockets ? sockets->DestroyPollGroup( pollGroup ) : false;
}

int Steam_GameServerNetworkingSockets_AcceptConnection( HSteamNetConnection connection )
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	return sockets ? static_cast<int>( sockets->AcceptConnection( connection ) ) : k_EResultInvalidState;
}

bool Steam_GameServerNetworkingSockets_CloseConnection( HSteamNetConnection connection, int reason, const char * debugMessage, bool enableLinger )
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	return sockets ? sockets->CloseConnection( connection, reason, debugMessage, enableLinger ) : false;
}

bool Steam_GameServerNetworkingSockets_CloseListenSocket( HSteamListenSocket listenSocket )
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	return sockets ? sockets->CloseListenSocket( listenSocket ) : false;
}

bool Steam_GameServerNetworkingSockets_SetConnectionPollGroup( HSteamNetConnection connection, HSteamNetPollGroup pollGroup )
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	return sockets ? sockets->SetConnectionPollGroup( connection, pollGroup ) : false;
}

int Steam_GameServerNetworkingSockets_SendMessageToConnectionString( HSteamNetConnection connection, const char * data, int sendFlags )
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	if ( !sockets || !data )
	{
		return k_EResultInvalidState;
	}
	const size_t dataSize = std::strlen( data );
	if ( dataSize > UINT32_MAX )
	{
		return k_EResultInvalidState;
	}
	return static_cast<int>( sockets->SendMessageToConnection( connection, data, static_cast<uint32>( dataSize ), sendFlags, nullptr ) );
}

int Steam_GameServerNetworkingSockets_SendMessageToConnectionBytes( HSteamNetConnection connection, const uint8_t * data, size_t dataSize, int sendFlags )
{
	auto *sockets = SteamAPI_SteamGameServerNetworkingSockets_SteamAPI();
	if ( !sockets || ( !data && dataSize > 0 ) || dataSize > UINT32_MAX )
	{
		return k_EResultInvalidState;
	}
	return static_cast<int>( sockets->SendMessageToConnection( connection, data, static_cast<uint32>( dataSize ), sendFlags, nullptr ) );
}

std::vector<std::string> Steam_GameServerNetworkingSockets_ReceiveMessagesOnConnectionStrings( HSteamNetConnection connection, int maxMessages )
{
	return ReceiveMessagesOnConnectionStrings( SteamAPI_SteamGameServerNetworkingSockets_SteamAPI(), connection, maxMessages );
}

SteamworksBytesVector Steam_GameServerNetworkingSockets_ReceiveMessagesOnConnectionBytes( HSteamNetConnection connection, int maxMessages )
{
	return ReceiveMessagesOnConnectionStrings( SteamAPI_SteamGameServerNetworkingSockets_SteamAPI(), connection, maxMessages );
}

std::vector<std::string> Steam_GameServerNetworkingSockets_ReceiveMessagesOnPollGroupStrings( HSteamNetPollGroup pollGroup, int maxMessages )
{
	return ReceiveMessagesOnPollGroupStrings( SteamAPI_SteamGameServerNetworkingSockets_SteamAPI(), pollGroup, maxMessages );
}

SteamworksBytesVector Steam_GameServerNetworkingSockets_ReceiveMessagesOnPollGroupBytes( HSteamNetPollGroup pollGroup, int maxMessages )
{
	return ReceiveMessagesOnPollGroupStrings( SteamAPI_SteamGameServerNetworkingSockets_SteamAPI(), pollGroup, maxMessages );
}

int Steam_NetworkingMessages_SendMessageToSteamIDBytes( uint64_steamid steamID, const uint8_t * data, size_t dataSize, int sendFlags, int remoteChannel )
{
	auto *messages = SteamAPI_SteamNetworkingMessages_SteamAPI();
	if ( !messages || ( !data && dataSize > 0 ) || dataSize > UINT32_MAX )
	{
		return k_EResultInvalidState;
	}

	const SteamNetworkingIdentity identity = IdentityForSteamID( steamID );
	return static_cast<int>(
		messages->SendMessageToUser( identity, data, static_cast<uint32>( dataSize ), sendFlags, remoteChannel )
	);
}

SteamworksBytesVector Steam_NetworkingMessages_ReceiveMessagesOnChannelBytes( int localChannel, int maxMessages )
{
	return ReceiveMessagesOnChannelBytes( SteamAPI_SteamNetworkingMessages_SteamAPI(), localChannel, maxMessages );
}

bool Steam_NetworkingMessages_AcceptSessionWithSteamID( uint64_steamid steamID )
{
	auto *messages = SteamAPI_SteamNetworkingMessages_SteamAPI();
	if ( !messages )
	{
		return false;
	}
	const SteamNetworkingIdentity identity = IdentityForSteamID( steamID );
	return messages->AcceptSessionWithUser( identity );
}

bool Steam_NetworkingMessages_CloseSessionWithSteamID( uint64_steamid steamID )
{
	auto *messages = SteamAPI_SteamNetworkingMessages_SteamAPI();
	if ( !messages )
	{
		return false;
	}
	const SteamNetworkingIdentity identity = IdentityForSteamID( steamID );
	return messages->CloseSessionWithUser( identity );
}

bool Steam_NetworkingMessages_CloseChannelWithSteamID( uint64_steamid steamID, int localChannel )
{
	auto *messages = SteamAPI_SteamNetworkingMessages_SteamAPI();
	if ( !messages )
	{
		return false;
	}
	const SteamNetworkingIdentity identity = IdentityForSteamID( steamID );
	return messages->CloseChannelWithUser( identity, localChannel );
}

int Steam_GameServerNetworkingMessages_SendMessageToSteamIDBytes( uint64_steamid steamID, const uint8_t * data, size_t dataSize, int sendFlags, int remoteChannel )
{
	auto *messages = SteamAPI_SteamGameServerNetworkingMessages_SteamAPI();
	if ( !messages || ( !data && dataSize > 0 ) || dataSize > UINT32_MAX )
	{
		return k_EResultInvalidState;
	}

	const SteamNetworkingIdentity identity = IdentityForSteamID( steamID );
	return static_cast<int>(
		messages->SendMessageToUser( identity, data, static_cast<uint32>( dataSize ), sendFlags, remoteChannel )
	);
}

SteamworksBytesVector Steam_GameServerNetworkingMessages_ReceiveMessagesOnChannelBytes( int localChannel, int maxMessages )
{
	return ReceiveMessagesOnChannelBytes( SteamAPI_SteamGameServerNetworkingMessages_SteamAPI(), localChannel, maxMessages );
}

bool Steam_GameServerNetworkingMessages_AcceptSessionWithSteamID( uint64_steamid steamID )
{
	auto *messages = SteamAPI_SteamGameServerNetworkingMessages_SteamAPI();
	if ( !messages )
	{
		return false;
	}
	const SteamNetworkingIdentity identity = IdentityForSteamID( steamID );
	return messages->AcceptSessionWithUser( identity );
}

bool Steam_GameServerNetworkingMessages_CloseSessionWithSteamID( uint64_steamid steamID )
{
	auto *messages = SteamAPI_SteamGameServerNetworkingMessages_SteamAPI();
	if ( !messages )
	{
		return false;
	}
	const SteamNetworkingIdentity identity = IdentityForSteamID( steamID );
	return messages->CloseSessionWithUser( identity );
}

bool Steam_GameServerNetworkingMessages_CloseChannelWithSteamID( uint64_steamid steamID, int localChannel )
{
	auto *messages = SteamAPI_SteamGameServerNetworkingMessages_SteamAPI();
	if ( !messages )
	{
		return false;
	}
	const SteamNetworkingIdentity identity = IdentityForSteamID( steamID );
	return messages->CloseChannelWithUser( identity, localChannel );
}

bool Steam_RemoteStorage_FileWriteBytes( const char * filename, const uint8_t * data, size_t dataSize )
{
	auto *storage = SteamAPI_SteamRemoteStorage();
	if ( !storage || !filename || ( !data && dataSize > 0 ) || dataSize > static_cast<size_t>( INT32_MAX ) )
	{
		return false;
	}
	return storage->FileWrite( filename, data, static_cast<int32>( dataSize ) );
}

SteamworksBytes Steam_RemoteStorage_FileReadBytes( const char * filename, int32 dataSize )
{
	auto *storage = SteamAPI_SteamRemoteStorage();
	if ( !storage || !filename || dataSize <= 0 )
	{
		return {};
	}

	SteamworksBytes data( static_cast<size_t>( dataSize ), '\0' );
	const int32 bytesRead = storage->FileRead( filename, data.data(), dataSize );
	if ( bytesRead < 0 || bytesRead > dataSize )
	{
		return {};
	}
	data.resize( static_cast<size_t>( bytesRead ) );
	return data;
}

SteamAPICall_t Steam_RemoteStorage_FileWriteAsyncBytes( const char * filename, const uint8_t * data, size_t dataSize )
{
	auto *storage = SteamAPI_SteamRemoteStorage();
	if ( !storage || !filename || ( !data && dataSize > 0 ) || dataSize > UINT32_MAX )
	{
		return k_uAPICallInvalid;
	}
	return storage->FileWriteAsync( filename, data, static_cast<uint32>( dataSize ) );
}

SteamworksBytes Steam_RemoteStorage_FileReadAsyncCompleteBytes( SteamAPICall_t readCall, uint32 dataSize )
{
	auto *storage = SteamAPI_SteamRemoteStorage();
	if ( !storage || dataSize == 0 )
	{
		return {};
	}

	SteamworksBytes data( static_cast<size_t>( dataSize ), '\0' );
	if ( !storage->FileReadAsyncComplete( readCall, data.data(), dataSize ) )
	{
		return {};
	}
	return data;
}

bool Steam_RemoteStorage_FileWriteStreamWriteChunkBytes( UGCFileWriteStreamHandle_t writeHandle, const uint8_t * data, size_t dataSize )
{
	auto *storage = SteamAPI_SteamRemoteStorage();
	if ( !storage || ( !data && dataSize > 0 ) || dataSize > static_cast<size_t>( INT32_MAX ) )
	{
		return false;
	}
	return storage->FileWriteStreamWriteChunk( writeHandle, data, static_cast<int32>( dataSize ) );
}

bool Steam_HTTP_SetHTTPRequestRawPostBodyBytes( HTTPRequestHandle request, const char * contentType, const uint8_t * data, size_t dataSize )
{
	return SetHTTPRequestRawPostBodyBytes( SteamAPI_SteamHTTP(), request, contentType, data, dataSize );
}

SteamworksBytes Steam_HTTP_GetHTTPResponseBodyDataBytes( HTTPRequestHandle request, uint32 dataSize )
{
	return GetHTTPResponseBodyDataBytes( SteamAPI_SteamHTTP(), request, dataSize );
}

SteamworksBytes Steam_HTTP_GetHTTPStreamingResponseBodyDataBytes( HTTPRequestHandle request, uint32 offset, uint32 dataSize )
{
	return GetHTTPStreamingResponseBodyDataBytes( SteamAPI_SteamHTTP(), request, offset, dataSize );
}

bool Steam_GameServerHTTP_SetHTTPRequestRawPostBodyBytes( HTTPRequestHandle request, const char * contentType, const uint8_t * data, size_t dataSize )
{
	return SetHTTPRequestRawPostBodyBytes( SteamAPI_SteamGameServerHTTP(), request, contentType, data, dataSize );
}

SteamworksBytes Steam_GameServerHTTP_GetHTTPResponseBodyDataBytes( HTTPRequestHandle request, uint32 dataSize )
{
	return GetHTTPResponseBodyDataBytes( SteamAPI_SteamGameServerHTTP(), request, dataSize );
}

SteamworksBytes Steam_GameServerHTTP_GetHTTPStreamingResponseBodyDataBytes( HTTPRequestHandle request, uint32 offset, uint32 dataSize )
{
	return GetHTTPStreamingResponseBodyDataBytes( SteamAPI_SteamGameServerHTTP(), request, offset, dataSize );
}

int Steam_NetworkingSend_Unreliable()
{
	return k_nSteamNetworkingSend_Unreliable;
}

int Steam_NetworkingSend_UnreliableNoDelay()
{
	return k_nSteamNetworkingSend_UnreliableNoDelay;
}

int Steam_NetworkingSend_Reliable()
{
	return k_nSteamNetworkingSend_Reliable;
}

int Steam_NetworkingSend_ReliableNoNagle()
{
	return k_nSteamNetworkingSend_ReliableNoNagle;
}

int Steam_NetworkingConnectionState_None()
{
	return k_ESteamNetworkingConnectionState_None;
}

int Steam_NetworkingConnectionState_Connecting()
{
	return k_ESteamNetworkingConnectionState_Connecting;
}

int Steam_NetworkingConnectionState_FindingRoute()
{
	return k_ESteamNetworkingConnectionState_FindingRoute;
}

int Steam_NetworkingConnectionState_Connected()
{
	return k_ESteamNetworkingConnectionState_Connected;
}

int Steam_NetworkingConnectionState_ClosedByPeer()
{
	return k_ESteamNetworkingConnectionState_ClosedByPeer;
}

int Steam_NetworkingConnectionState_ProblemDetectedLocally()
{
	return k_ESteamNetworkingConnectionState_ProblemDetectedLocally;
}

int Steam_NetConnectionEnd_AppGeneric()
{
	return k_ESteamNetConnectionEnd_App_Generic;
}

int Steam_NetConnectionEnd_AppExceptionGeneric()
{
	return k_ESteamNetConnectionEnd_AppException_Generic;
}

SteamAPICall_t Steam_Lobby_RequestList()
{
	return g_lobbyAsyncState.RequestList();
}

bool Steam_Lobby_IsListPending()
{
	return g_lobbyAsyncState.IsListPending();
}

bool Steam_Lobby_IsListComplete()
{
	return g_lobbyAsyncState.IsListComplete();
}

bool Steam_Lobby_ListHadIOFailure()
{
	return g_lobbyAsyncState.ListHadIOFailure();
}

uint32 Steam_Lobby_GetListResultCount()
{
	return g_lobbyAsyncState.GetLobbyCount();
}

uint64_steamid Steam_Lobby_GetListLobbyByIndex( int index )
{
	if ( index < 0 || static_cast<uint32>( index ) >= g_lobbyAsyncState.GetLobbyCount() )
	{
		return 0;
	}
	ISteamMatchmaking *matchmaking = SteamMatchmaking();
	return matchmaking ? matchmaking->GetLobbyByIndex( index ).ConvertToUint64() : 0;
}

std::string Steam_Lobby_GetListLobbyNameByIndex( int index )
{
	const uint64_steamid lobbyID = Steam_Lobby_GetListLobbyByIndex( index );
	if ( lobbyID == 0 )
	{
		return {};
	}
	ISteamMatchmaking *matchmaking = SteamMatchmaking();
	if ( !matchmaking )
	{
		return {};
	}
	const char *name = matchmaking->GetLobbyData( CSteamID( lobbyID ), "name" );
	return name ? std::string( name ) : std::string();
}

void Steam_Lobby_ClearAsyncState()
{
	g_lobbyAsyncState.Clear();
}

std::vector<std::string> Steam_Lobby_GetDataEntries( uint64_steamid lobbyID )
{
	std::vector<std::string> result;
	ISteamMatchmaking *matchmaking = SteamMatchmaking();
	if ( !matchmaking || lobbyID == 0 )
	{
		return result;
	}

	const int count = matchmaking->GetLobbyDataCount( CSteamID( lobbyID ) );
	result.reserve( static_cast<size_t>( std::max( count, 0 ) ) );
	for ( int index = 0; index < count; ++index )
	{
		char key[256] = { 0 };
		char value[4096] = { 0 };
		if ( matchmaking->GetLobbyDataByIndex( CSteamID( lobbyID ), index, key, sizeof( key ), value, sizeof( value ) ) )
		{
			result.emplace_back( std::string( key ) + "=" + std::string( value ) );
		}
	}
	return result;
}

SteamAPICall_t Steam_Lobby_Create( int lobbyType, int maxMembers )
{
	return g_lobbyAsyncState.Create( lobbyType, maxMembers );
}

bool Steam_Lobby_IsCreatePending()
{
	return g_lobbyAsyncState.IsCreatePending();
}

bool Steam_Lobby_IsCreateComplete()
{
	return g_lobbyAsyncState.IsCreateComplete();
}

bool Steam_Lobby_CreateHadIOFailure()
{
	return g_lobbyAsyncState.CreateHadIOFailure();
}

uint64_steamid Steam_Lobby_GetCreatedLobbyID()
{
	return g_lobbyAsyncState.GetCreatedLobbyID();
}

int Steam_Lobby_GetCreateResult()
{
	return g_lobbyAsyncState.GetCreateResult();
}

bool Steam_Lobby_CreateSucceeded()
{
	return g_lobbyAsyncState.IsCreateComplete()
		&& !g_lobbyAsyncState.CreateHadIOFailure()
		&& g_lobbyAsyncState.GetCreateResult() == k_EResultOK;
}

SteamAPICall_t Steam_Lobby_Join( uint64_steamid lobbyID )
{
	return g_lobbyAsyncState.Join( lobbyID );
}

bool Steam_Lobby_IsJoinPending()
{
	return g_lobbyAsyncState.IsJoinPending();
}

bool Steam_Lobby_IsJoinComplete()
{
	return g_lobbyAsyncState.IsJoinComplete();
}

bool Steam_Lobby_JoinHadIOFailure()
{
	return g_lobbyAsyncState.JoinHadIOFailure();
}

uint64_steamid Steam_Lobby_GetJoinedLobbyID()
{
	return g_lobbyAsyncState.GetJoinedLobbyID();
}

uint32 Steam_Lobby_GetJoinResponse()
{
	return g_lobbyAsyncState.GetJoinResponse();
}

bool Steam_Lobby_JoinSucceeded()
{
	return g_lobbyAsyncState.IsJoinComplete()
		&& !g_lobbyAsyncState.JoinHadIOFailure()
		&& g_lobbyAsyncState.GetJoinResponse() == k_EChatRoomEnterResponseSuccess;
}

int Steam_Lobby_ChatRoomEnterResponseSuccess()
{
	return k_EChatRoomEnterResponseSuccess;
}

void Steam_Lobby_EnableChatCallbacks()
{
	SelectCallbackDispatchMode( k_CallbackDispatchModeAutomatic, "Steam_Lobby_EnableChatCallbacks" );
	g_lobbyChatQueue.Register();
}

void Steam_Lobby_ClearChatMessages()
{
	g_lobbyChatQueue.Clear();
}

bool Steam_Lobby_SendChatMessage( uint64_steamid lobbyID, const char * message )
{
	ISteamMatchmaking *matchmaking = SteamMatchmaking();
	if ( !matchmaking || lobbyID == 0 || !message )
	{
		return false;
	}
	const size_t messageSize = std::strlen( message );
	if ( messageSize >= static_cast<size_t>( INT_MAX ) )
	{
		return false;
	}
	return matchmaking->SendLobbyChatMsg( CSteamID( lobbyID ), message, static_cast<int>( messageSize + 1 ) );
}

std::vector<std::string> Steam_Lobby_PollChatMessages( int maxMessages )
{
	return g_lobbyChatQueue.PopMessages( maxMessages );
}

HServerQuery Steam_MatchmakingServers_PingServer( uint32 ip, uint16 port )
{
	return g_matchmakingPingResponse.PingServer( ip, port );
}

bool Steam_MatchmakingServers_IsPingPending()
{
	return g_matchmakingPingResponse.IsPending();
}

bool Steam_MatchmakingServers_IsPingComplete()
{
	return g_matchmakingPingResponse.IsComplete();
}

bool Steam_MatchmakingServers_PingFailed()
{
	return g_matchmakingPingResponse.Failed();
}

bool Steam_MatchmakingServers_PingSucceeded()
{
	return g_matchmakingPingResponse.Succeeded();
}

std::string Steam_MatchmakingServers_GetPingServer()
{
	return g_matchmakingPingResponse.GetServer();
}

void Steam_MatchmakingServers_ClearPingResult()
{
	g_matchmakingPingResponse.Clear();
}

HServerQuery Steam_MatchmakingServers_ServerFriends( uint32 ip, uint16 port )
{
	return g_matchmakingServerFriendsResponse.ServerFriends( ip, port );
}

bool Steam_MatchmakingServers_IsServerFriendsPending()
{
	return g_matchmakingServerFriendsResponse.IsPending();
}

bool Steam_MatchmakingServers_IsServerFriendsComplete()
{
	return g_matchmakingServerFriendsResponse.IsComplete();
}

bool Steam_MatchmakingServers_ServerFriendsFailed()
{
	return g_matchmakingServerFriendsResponse.Failed();
}

bool Steam_MatchmakingServers_ServerFriendsSucceeded()
{
	return g_matchmakingServerFriendsResponse.Succeeded();
}

std::vector<std::string> Steam_MatchmakingServers_GetServerFriends()
{
	return g_matchmakingServerFriendsResponse.GetFriends();
}

void Steam_MatchmakingServers_ClearServerFriendsResult()
{
	g_matchmakingServerFriendsResponse.Clear();
}

int Steam_FriendFlagImmediate()
{
	return k_EFriendFlagImmediate;
}

int Steam_FriendFlagOnGameServer()
{
	return k_EFriendFlagOnGameServer;
}

int Steam_FriendFlagAll()
{
	return k_EFriendFlagAll;
}

int Steam_LobbyTypePrivate()
{
	return k_ELobbyTypePrivate;
}

int Steam_LobbyTypeFriendsOnly()
{
	return k_ELobbyTypeFriendsOnly;
}

int Steam_LobbyTypePublic()
{
	return k_ELobbyTypePublic;
}

int Steam_LobbyTypeInvisible()
{
	return k_ELobbyTypeInvisible;
}

int Steam_LobbyTypePrivateUnique()
{
	return k_ELobbyTypePrivateUnique;
}

int Steam_LobbyComparisonEqual()
{
	return k_ELobbyComparisonEqual;
}

int Steam_PersonaStateOffline()
{
	return k_EPersonaStateOffline;
}

int Steam_PersonaStateOnline()
{
	return k_EPersonaStateOnline;
}

int Steam_PersonaStateBusy()
{
	return k_EPersonaStateBusy;
}

int Steam_PersonaStateAway()
{
	return k_EPersonaStateAway;
}

int Steam_PersonaStateSnooze()
{
	return k_EPersonaStateSnooze;
}

int Steam_PersonaStateLookingToTrade()
{
	return k_EPersonaStateLookingToTrade;
}

int Steam_PersonaStateLookingToPlay()
{
	return k_EPersonaStateLookingToPlay;
}

int Steam_PersonaStateInvisible()
{
	return k_EPersonaStateInvisible;
}

int Steam_Friends_GetFriendCountImmediate()
{
	ISteamFriends *friends = SteamFriends();
	return friends ? friends->GetFriendCount( k_EFriendFlagImmediate ) : 0;
}

uint64_steamid Steam_Friends_GetFriendByIndexImmediate( int index )
{
	ISteamFriends *friends = SteamFriends();
	if ( !friends || index < 0 )
	{
		return 0;
	}
	return friends->GetFriendByIndex( index, k_EFriendFlagImmediate ).ConvertToUint64();
}

bool Steam_Friends_GetFriendGamePlayedInfo( uint64_steamid friendID )
{
	FriendGameInfo_t info{};
	return GetFriendGameInfo( friendID, &info );
}

uint64_gameid Steam_Friends_GetFriendGameID( uint64_steamid friendID )
{
	FriendGameInfo_t info{};
	return GetFriendGameInfo( friendID, &info ) ? info.m_gameID.ToUint64() : 0;
}

AppId_t Steam_Friends_GetFriendGameAppID( uint64_steamid friendID )
{
	FriendGameInfo_t info{};
	return GetFriendGameInfo( friendID, &info ) ? static_cast<AppId_t>( info.m_gameID.AppID() ) : k_uAppIdInvalid;
}

uint32 Steam_Friends_GetFriendGameIP( uint64_steamid friendID )
{
	FriendGameInfo_t info{};
	return GetFriendGameInfo( friendID, &info ) ? info.m_unGameIP : 0;
}

uint16 Steam_Friends_GetFriendGamePort( uint64_steamid friendID )
{
	FriendGameInfo_t info{};
	return GetFriendGameInfo( friendID, &info ) ? info.m_usGamePort : 0;
}

uint16 Steam_Friends_GetFriendGameQueryPort( uint64_steamid friendID )
{
	FriendGameInfo_t info{};
	return GetFriendGameInfo( friendID, &info ) ? info.m_usQueryPort : 0;
}

uint64_steamid Steam_Friends_GetFriendGameLobbyID( uint64_steamid friendID )
{
	FriendGameInfo_t info{};
	return GetFriendGameInfo( friendID, &info ) ? info.m_steamIDLobby.ConvertToUint64() : 0;
}

bool Steam_Friends_IsFriendInCurrentGame( uint64_steamid friendID )
{
	ISteamUtils *utils = SteamUtils();
	if ( !utils )
	{
		return false;
	}

	const AppId_t friendAppID = Steam_Friends_GetFriendGameAppID( friendID );
	return friendAppID != k_uAppIdInvalid && friendAppID == utils->GetAppID();
}

bool Steam_Apps_BIsAppInstalled( AppId_t appID )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsAppInstalled( self, appID );
}

bool Steam_Apps_BIsCybercafe(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsCybercafe( self );
}

bool Steam_Apps_BIsDlcInstalled( AppId_t appID )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsDlcInstalled( self, appID );
}

bool Steam_Apps_BIsLowViolence(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsLowViolence( self );
}

bool Steam_Apps_BIsSubscribed(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsSubscribed( self );
}

bool Steam_Apps_BIsSubscribedApp( AppId_t appID )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsSubscribedApp( self, appID );
}

bool Steam_Apps_BIsSubscribedFromFamilySharing(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsSubscribedFromFamilySharing( self );
}

bool Steam_Apps_BIsSubscribedFromFreeWeekend(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsSubscribedFromFreeWeekend( self );
}

bool Steam_Apps_BIsVACBanned(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_BIsVACBanned( self );
}

int Steam_Apps_GetAppBuildId(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_GetAppBuildId( self );
}

uint64_steamid Steam_Apps_GetAppOwner(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_GetAppOwner( self );
}

const char * Steam_Apps_GetAvailableGameLanguages(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_GetAvailableGameLanguages( self );
}

const char * Steam_Apps_GetCurrentGameLanguage(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_GetCurrentGameLanguage( self );
}

int Steam_Apps_GetDLCCount(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_GetDLCCount( self );
}

uint32 Steam_Apps_GetEarliestPurchaseUnixTime( AppId_t nAppID )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime( self, nAppID );
}

SteamAPICall_t Steam_Apps_GetFileDetails( const char * pszFileName )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_GetFileDetails( self, pszFileName );
}

const char * Steam_Apps_GetLaunchQueryParam( const char * pchKey )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_GetLaunchQueryParam( self, pchKey );
}

void Steam_Apps_InstallDLC( AppId_t nAppID )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamApps_InstallDLC( self, nAppID );
}

bool Steam_Apps_MarkContentCorrupt( bool bMissingFilesOnly )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_MarkContentCorrupt( self, bMissingFilesOnly );
}

void Steam_Apps_RequestAllProofOfPurchaseKeys(  )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamApps_RequestAllProofOfPurchaseKeys( self );
}

void Steam_Apps_RequestAppProofOfPurchaseKey( AppId_t nAppID )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamApps_RequestAppProofOfPurchaseKey( self, nAppID );
}

bool Steam_Apps_SetActiveBeta( const char * pchBetaName )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_SetActiveBeta( self, pchBetaName );
}

bool Steam_Apps_SetDlcContext( AppId_t nAppID )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamApps_SetDlcContext( self, nAppID );
}

void Steam_Apps_SetGamePerformanceSetting( EGamePerformanceSetting setting )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamApps_SetGamePerformanceSetting( self, setting );
}

void Steam_Apps_SetGameRenderResolution( uint32 unWidth, uint32 unHeight )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamApps_SetGameRenderResolution( self, unWidth, unHeight );
}

void Steam_Apps_UninstallDLC( AppId_t nAppID )
{
	auto *self = SteamAPI_SteamApps_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamApps_UninstallDLC( self, nAppID );
}

void Steam_Controller_ActivateActionSet( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_ActivateActionSet( self, controllerHandle, actionSetHandle );
}

void Steam_Controller_ActivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_ActivateActionSetLayer( self, controllerHandle, actionSetLayerHandle );
}

void Steam_Controller_DeactivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_DeactivateActionSetLayer( self, controllerHandle, actionSetLayerHandle );
}

void Steam_Controller_DeactivateAllActionSetLayers( ControllerHandle_t controllerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_DeactivateAllActionSetLayers( self, controllerHandle );
}

EControllerActionOrigin Steam_Controller_GetActionOriginFromXboxOrigin( ControllerHandle_t controllerHandle, EXboxOrigin eOrigin )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetActionOriginFromXboxOrigin( self, controllerHandle, eOrigin );
}

ControllerActionSetHandle_t Steam_Controller_GetActionSetHandle( const char * pszActionSetName )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetActionSetHandle( self, pszActionSetName );
}

InputAnalogActionData_t Steam_Controller_GetAnalogActionData( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetAnalogActionData( self, controllerHandle, analogActionHandle );
}

ControllerAnalogActionHandle_t Steam_Controller_GetAnalogActionHandle( const char * pszActionName )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetAnalogActionHandle( self, pszActionName );
}

ControllerHandle_t Steam_Controller_GetControllerForGamepadIndex( int nIndex )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetControllerForGamepadIndex( self, nIndex );
}

ControllerActionSetHandle_t Steam_Controller_GetCurrentActionSet( ControllerHandle_t controllerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetCurrentActionSet( self, controllerHandle );
}

InputDigitalActionData_t Steam_Controller_GetDigitalActionData( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetDigitalActionData( self, controllerHandle, digitalActionHandle );
}

ControllerDigitalActionHandle_t Steam_Controller_GetDigitalActionHandle( const char * pszActionName )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetDigitalActionHandle( self, pszActionName );
}

int Steam_Controller_GetGamepadIndexForController( ControllerHandle_t ulControllerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetGamepadIndexForController( self, ulControllerHandle );
}

const char * Steam_Controller_GetGlyphForActionOrigin( EControllerActionOrigin eOrigin )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetGlyphForActionOrigin( self, eOrigin );
}

const char * Steam_Controller_GetGlyphForXboxOrigin( EXboxOrigin eOrigin )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetGlyphForXboxOrigin( self, eOrigin );
}

ESteamInputType Steam_Controller_GetInputTypeForHandle( ControllerHandle_t controllerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetInputTypeForHandle( self, controllerHandle );
}

InputMotionData_t Steam_Controller_GetMotionData( ControllerHandle_t controllerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetMotionData( self, controllerHandle );
}

const char * Steam_Controller_GetStringForActionOrigin( EControllerActionOrigin eOrigin )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetStringForActionOrigin( self, eOrigin );
}

const char * Steam_Controller_GetStringForXboxOrigin( EXboxOrigin eOrigin )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_GetStringForXboxOrigin( self, eOrigin );
}

bool Steam_Controller_Init(  )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_Init( self );
}

void Steam_Controller_RunFrame(  )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_RunFrame( self );
}

void Steam_Controller_SetLEDColor( ControllerHandle_t controllerHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_SetLEDColor( self, controllerHandle, nColorR, nColorG, nColorB, nFlags );
}

bool Steam_Controller_ShowBindingPanel( ControllerHandle_t controllerHandle )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_ShowBindingPanel( self, controllerHandle );
}

bool Steam_Controller_Shutdown(  )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_Shutdown( self );
}

void Steam_Controller_StopAnalogActionMomentum( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t eAction )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_StopAnalogActionMomentum( self, controllerHandle, eAction );
}

EControllerActionOrigin Steam_Controller_TranslateActionOrigin( ESteamInputType eDestinationInputType, EControllerActionOrigin eSourceOrigin )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamController_TranslateActionOrigin( self, eDestinationInputType, eSourceOrigin );
}

void Steam_Controller_TriggerHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_TriggerHapticPulse( self, controllerHandle, eTargetPad, usDurationMicroSec );
}

void Steam_Controller_TriggerRepeatedHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_TriggerRepeatedHapticPulse( self, controllerHandle, eTargetPad, usDurationMicroSec, usOffMicroSec, unRepeat, nFlags );
}

void Steam_Controller_TriggerVibration( ControllerHandle_t controllerHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed )
{
	auto *self = SteamAPI_SteamController_v008();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamController_TriggerVibration( self, controllerHandle, usLeftSpeed, usRightSpeed );
}

void Steam_Friends_ActivateGameOverlay( const char * pchDialog )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_ActivateGameOverlay( self, pchDialog );
}

void Steam_Friends_ActivateGameOverlayInviteDialog( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog( self, steamIDLobby );
}

void Steam_Friends_ActivateGameOverlayInviteDialogConnectString( const char * pchConnectString )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialogConnectString( self, pchConnectString );
}

void Steam_Friends_ActivateGameOverlayRemotePlayTogetherInviteDialog( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_ActivateGameOverlayRemotePlayTogetherInviteDialog( self, steamIDLobby );
}

void Steam_Friends_ActivateGameOverlayToStore( AppId_t nAppID, EOverlayToStoreFlag eFlag )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_ActivateGameOverlayToStore( self, nAppID, eFlag );
}

void Steam_Friends_ActivateGameOverlayToUser( const char * pchDialog, uint64_steamid steamID )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_ActivateGameOverlayToUser( self, pchDialog, steamID );
}

void Steam_Friends_ActivateGameOverlayToWebPage( const char * pchURL, EActivateGameOverlayToWebPageMode eMode )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage( self, pchURL, eMode );
}

bool Steam_Friends_BHasEquippedProfileItem( uint64_steamid steamID, ECommunityProfileItemType itemType )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_BHasEquippedProfileItem( self, steamID, itemType );
}

void Steam_Friends_ClearRichPresence(  )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_ClearRichPresence( self );
}

bool Steam_Friends_CloseClanChatWindowInSteam( uint64_steamid steamIDClanChat )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_CloseClanChatWindowInSteam( self, steamIDClanChat );
}

SteamAPICall_t Steam_Friends_EnumerateFollowingList( uint32 unStartIndex )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_EnumerateFollowingList( self, unStartIndex );
}

uint64_steamid Steam_Friends_GetChatMemberByIndex( uint64_steamid steamIDClan, int iUser )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetChatMemberByIndex( self, steamIDClan, iUser );
}

uint64_steamid Steam_Friends_GetClanByIndex( int iClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetClanByIndex( self, iClan );
}

int Steam_Friends_GetClanChatMemberCount( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetClanChatMemberCount( self, steamIDClan );
}

int Steam_Friends_GetClanCount(  )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetClanCount( self );
}

const char * Steam_Friends_GetClanName( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetClanName( self, steamIDClan );
}

uint64_steamid Steam_Friends_GetClanOfficerByIndex( uint64_steamid steamIDClan, int iOfficer )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetClanOfficerByIndex( self, steamIDClan, iOfficer );
}

int Steam_Friends_GetClanOfficerCount( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetClanOfficerCount( self, steamIDClan );
}

uint64_steamid Steam_Friends_GetClanOwner( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetClanOwner( self, steamIDClan );
}

const char * Steam_Friends_GetClanTag( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetClanTag( self, steamIDClan );
}

uint64_steamid Steam_Friends_GetCoplayFriend( int iCoplayFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetCoplayFriend( self, iCoplayFriend );
}

int Steam_Friends_GetCoplayFriendCount(  )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetCoplayFriendCount( self );
}

SteamAPICall_t Steam_Friends_GetFollowerCount( uint64_steamid steamID )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFollowerCount( self, steamID );
}

uint64_steamid Steam_Friends_GetFriendByIndex( int iFriend, int iFriendFlags )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendByIndex( self, iFriend, iFriendFlags );
}

AppId_t Steam_Friends_GetFriendCoplayGame( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendCoplayGame( self, steamIDFriend );
}

int Steam_Friends_GetFriendCoplayTime( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendCoplayTime( self, steamIDFriend );
}

int Steam_Friends_GetFriendCount( int iFriendFlags )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendCount( self, iFriendFlags );
}

int Steam_Friends_GetFriendCountFromSource( uint64_steamid steamIDSource )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendCountFromSource( self, steamIDSource );
}

uint64_steamid Steam_Friends_GetFriendFromSourceByIndex( uint64_steamid steamIDSource, int iFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendFromSourceByIndex( self, steamIDSource, iFriend );
}

const char * Steam_Friends_GetFriendPersonaName( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendPersonaName( self, steamIDFriend );
}

const char * Steam_Friends_GetFriendPersonaNameHistory( uint64_steamid steamIDFriend, int iPersonaName )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendPersonaNameHistory( self, steamIDFriend, iPersonaName );
}

EPersonaState Steam_Friends_GetFriendPersonaState( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendPersonaState( self, steamIDFriend );
}

EFriendRelationship Steam_Friends_GetFriendRelationship( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendRelationship( self, steamIDFriend );
}

const char * Steam_Friends_GetFriendRichPresence( uint64_steamid steamIDFriend, const char * pchKey )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendRichPresence( self, steamIDFriend, pchKey );
}

const char * Steam_Friends_GetFriendRichPresenceKeyByIndex( uint64_steamid steamIDFriend, int iKey )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex( self, steamIDFriend, iKey );
}

int Steam_Friends_GetFriendRichPresenceKeyCount( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount( self, steamIDFriend );
}

int Steam_Friends_GetFriendSteamLevel( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendSteamLevel( self, steamIDFriend );
}

int Steam_Friends_GetFriendsGroupCount(  )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendsGroupCount( self );
}

FriendsGroupID_t Steam_Friends_GetFriendsGroupIDByIndex( int iFG )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendsGroupIDByIndex( self, iFG );
}

int Steam_Friends_GetFriendsGroupMembersCount( FriendsGroupID_t friendsGroupID )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendsGroupMembersCount( self, friendsGroupID );
}

const char * Steam_Friends_GetFriendsGroupName( FriendsGroupID_t friendsGroupID )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetFriendsGroupName( self, friendsGroupID );
}

int Steam_Friends_GetLargeFriendAvatar( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetLargeFriendAvatar( self, steamIDFriend );
}

int Steam_Friends_GetMediumFriendAvatar( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetMediumFriendAvatar( self, steamIDFriend );
}

int Steam_Friends_GetNumChatsWithUnreadPriorityMessages(  )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetNumChatsWithUnreadPriorityMessages( self );
}

const char * Steam_Friends_GetPersonaName(  )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetPersonaName( self );
}

EPersonaState Steam_Friends_GetPersonaState(  )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetPersonaState( self );
}

const char * Steam_Friends_GetPlayerNickname( uint64_steamid steamIDPlayer )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetPlayerNickname( self, steamIDPlayer );
}

const char * Steam_Friends_GetProfileItemPropertyString( uint64_steamid steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetProfileItemPropertyString( self, steamID, itemType, prop );
}

uint32 Steam_Friends_GetProfileItemPropertyUint( uint64_steamid steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetProfileItemPropertyUint( self, steamID, itemType, prop );
}

int Steam_Friends_GetSmallFriendAvatar( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_GetSmallFriendAvatar( self, steamIDFriend );
}

bool Steam_Friends_HasFriend( uint64_steamid steamIDFriend, int iFriendFlags )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_HasFriend( self, steamIDFriend, iFriendFlags );
}

bool Steam_Friends_InviteUserToGame( uint64_steamid steamIDFriend, const char * pchConnectString )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_InviteUserToGame( self, steamIDFriend, pchConnectString );
}

bool Steam_Friends_IsClanChatAdmin( uint64_steamid steamIDClanChat, uint64_steamid steamIDUser )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_IsClanChatAdmin( self, steamIDClanChat, steamIDUser );
}

bool Steam_Friends_IsClanChatWindowOpenInSteam( uint64_steamid steamIDClanChat )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_IsClanChatWindowOpenInSteam( self, steamIDClanChat );
}

bool Steam_Friends_IsClanOfficialGameGroup( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_IsClanOfficialGameGroup( self, steamIDClan );
}

bool Steam_Friends_IsClanPublic( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_IsClanPublic( self, steamIDClan );
}

SteamAPICall_t Steam_Friends_IsFollowing( uint64_steamid steamID )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_IsFollowing( self, steamID );
}

bool Steam_Friends_IsUserInSource( uint64_steamid steamIDUser, uint64_steamid steamIDSource )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_IsUserInSource( self, steamIDUser, steamIDSource );
}

SteamAPICall_t Steam_Friends_JoinClanChatRoom( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_JoinClanChatRoom( self, steamIDClan );
}

bool Steam_Friends_LeaveClanChatRoom( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_LeaveClanChatRoom( self, steamIDClan );
}

bool Steam_Friends_OpenClanChatWindowInSteam( uint64_steamid steamIDClanChat )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_OpenClanChatWindowInSteam( self, steamIDClanChat );
}

bool Steam_Friends_RegisterProtocolInOverlayBrowser( const char * pchProtocol )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_RegisterProtocolInOverlayBrowser( self, pchProtocol );
}

bool Steam_Friends_ReplyToFriendMessage( uint64_steamid steamIDFriend, const char * pchMsgToSend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_ReplyToFriendMessage( self, steamIDFriend, pchMsgToSend );
}

SteamAPICall_t Steam_Friends_RequestClanOfficerList( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_RequestClanOfficerList( self, steamIDClan );
}

SteamAPICall_t Steam_Friends_RequestEquippedProfileItems( uint64_steamid steamID )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_RequestEquippedProfileItems( self, steamID );
}

void Steam_Friends_RequestFriendRichPresence( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_RequestFriendRichPresence( self, steamIDFriend );
}

bool Steam_Friends_RequestUserInformation( uint64_steamid steamIDUser, bool bRequireNameOnly )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_RequestUserInformation( self, steamIDUser, bRequireNameOnly );
}

bool Steam_Friends_SendClanChatMessage( uint64_steamid steamIDClanChat, const char * pchText )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_SendClanChatMessage( self, steamIDClanChat, pchText );
}

void Steam_Friends_SetInGameVoiceSpeaking( uint64_steamid steamIDUser, bool bSpeaking )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_SetInGameVoiceSpeaking( self, steamIDUser, bSpeaking );
}

bool Steam_Friends_SetListenForFriendsMessages( bool bInterceptEnabled )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_SetListenForFriendsMessages( self, bInterceptEnabled );
}

void Steam_Friends_SetPlayedWith( uint64_steamid steamIDUserPlayedWith )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamFriends_SetPlayedWith( self, steamIDUserPlayedWith );
}

bool Steam_Friends_SetRichPresence( const char * pchKey, const char * pchValue )
{
	auto *self = SteamAPI_SteamFriends_v018();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamFriends_SetRichPresence( self, pchKey, pchValue );
}

bool Steam_GameServerStats_ClearUserAchievement( uint64_steamid steamIDUser, const char * pchName )
{
	auto *self = SteamAPI_SteamGameServerStats_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServerStats_ClearUserAchievement( self, steamIDUser, pchName );
}

SteamAPICall_t Steam_GameServerStats_RequestUserStats( uint64_steamid steamIDUser )
{
	auto *self = SteamAPI_SteamGameServerStats_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServerStats_RequestUserStats( self, steamIDUser );
}

bool Steam_GameServerStats_SetUserAchievement( uint64_steamid steamIDUser, const char * pchName )
{
	auto *self = SteamAPI_SteamGameServerStats_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServerStats_SetUserAchievement( self, steamIDUser, pchName );
}

bool Steam_GameServerStats_SetUserStat( uint64_steamid steamIDUser, const char * pchName, int32 nData )
{
	auto *self = SteamAPI_SteamGameServerStats_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServerStats_SetUserStatInt32( self, steamIDUser, pchName, nData );
}

bool Steam_GameServerStats_SetUserStat( uint64_steamid steamIDUser, const char * pchName, float fData )
{
	auto *self = SteamAPI_SteamGameServerStats_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServerStats_SetUserStatFloat( self, steamIDUser, pchName, fData );
}

SteamAPICall_t Steam_GameServerStats_StoreUserStats( uint64_steamid steamIDUser )
{
	auto *self = SteamAPI_SteamGameServerStats_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServerStats_StoreUserStats( self, steamIDUser );
}

bool Steam_GameServerStats_UpdateUserAvgRateStat( uint64_steamid steamIDUser, const char * pchName, float flCountThisSession, double dSessionLength )
{
	auto *self = SteamAPI_SteamGameServerStats_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServerStats_UpdateUserAvgRateStat( self, steamIDUser, pchName, flCountThisSession, dSessionLength );
}

SteamAPICall_t Steam_GameServer_AssociateWithClan( uint64_steamid steamIDClan )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_AssociateWithClan( self, steamIDClan );
}

bool Steam_GameServer_BLoggedOn(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_BLoggedOn( self );
}

bool Steam_GameServer_BSecure(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_BSecure( self );
}

bool Steam_GameServer_BUpdateUserData( uint64_steamid steamIDUser, const char * pchPlayerName, uint32 uScore )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_BUpdateUserData( self, steamIDUser, pchPlayerName, uScore );
}

void Steam_GameServer_CancelAuthTicket( HAuthTicket hAuthTicket )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_CancelAuthTicket( self, hAuthTicket );
}

void Steam_GameServer_ClearAllKeyValues(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_ClearAllKeyValues( self );
}

SteamAPICall_t Steam_GameServer_ComputeNewPlayerCompatibility( uint64_steamid steamIDNewPlayer )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_ComputeNewPlayerCompatibility( self, steamIDNewPlayer );
}

uint64_steamid Steam_GameServer_CreateUnauthenticatedUserConnection(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_CreateUnauthenticatedUserConnection( self );
}

void Steam_GameServer_EndAuthSession( uint64_steamid steamID )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_EndAuthSession( self, steamID );
}

void Steam_GameServer_GetGameplayStats(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_GetGameplayStats( self );
}

SteamIPAddress_t Steam_GameServer_GetPublicIP(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_GetPublicIP( self );
}

SteamAPICall_t Steam_GameServer_GetServerReputation(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_GetServerReputation( self );
}

uint64_steamid Steam_GameServer_GetSteamID(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_GetSteamID( self );
}

void Steam_GameServer_LogOff(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_LogOff( self );
}

void Steam_GameServer_LogOn( const char * pszToken )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_LogOn( self, pszToken );
}

void Steam_GameServer_LogOnAnonymous(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_LogOnAnonymous( self );
}

bool Steam_GameServer_RequestUserGroupStatus( uint64_steamid steamIDUser, uint64_steamid steamIDGroup )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_RequestUserGroupStatus( self, steamIDUser, steamIDGroup );
}

void Steam_GameServer_SendUserDisconnect_DEPRECATED( uint64_steamid steamIDUser )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SendUserDisconnect_DEPRECATED( self, steamIDUser );
}

void Steam_GameServer_SetAdvertiseServerActive( bool bActive )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetAdvertiseServerActive( self, bActive );
}

void Steam_GameServer_SetBotPlayerCount( int cBotplayers )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetBotPlayerCount( self, cBotplayers );
}

void Steam_GameServer_SetDedicatedServer( bool bDedicated )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetDedicatedServer( self, bDedicated );
}

void Steam_GameServer_SetGameData( const char * pchGameData )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetGameData( self, pchGameData );
}

void Steam_GameServer_SetGameDescription( const char * pszGameDescription )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetGameDescription( self, pszGameDescription );
}

void Steam_GameServer_SetGameTags( const char * pchGameTags )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetGameTags( self, pchGameTags );
}

void Steam_GameServer_SetKeyValue( const char * pKey, const char * pValue )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetKeyValue( self, pKey, pValue );
}

void Steam_GameServer_SetMapName( const char * pszMapName )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetMapName( self, pszMapName );
}

void Steam_GameServer_SetMaxPlayerCount( int cPlayersMax )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetMaxPlayerCount( self, cPlayersMax );
}

void Steam_GameServer_SetModDir( const char * pszModDir )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetModDir( self, pszModDir );
}

void Steam_GameServer_SetPasswordProtected( bool bPasswordProtected )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetPasswordProtected( self, bPasswordProtected );
}

void Steam_GameServer_SetProduct( const char * pszProduct )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetProduct( self, pszProduct );
}

void Steam_GameServer_SetRegion( const char * pszRegion )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetRegion( self, pszRegion );
}

void Steam_GameServer_SetServerName( const char * pszServerName )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetServerName( self, pszServerName );
}

void Steam_GameServer_SetSpectatorPort( uint16 unSpectatorPort )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetSpectatorPort( self, unSpectatorPort );
}

void Steam_GameServer_SetSpectatorServerName( const char * pszSpectatorServerName )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamGameServer_SetSpectatorServerName( self, pszSpectatorServerName );
}

EUserHasLicenseForAppResult Steam_GameServer_UserHasLicenseForApp( uint64_steamid steamID, AppId_t appID )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_UserHasLicenseForApp( self, steamID, appID );
}

bool Steam_GameServer_WasRestartRequested(  )
{
	auto *self = SteamAPI_SteamGameServer_v015();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamGameServer_WasRestartRequested( self );
}

void Steam_HTMLSurface_AddHeader( HHTMLBrowser unBrowserHandle, const char * pchKey, const char * pchValue )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_AddHeader( self, unBrowserHandle, pchKey, pchValue );
}

void Steam_HTMLSurface_AllowStartRequest( HHTMLBrowser unBrowserHandle, bool bAllowed )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_AllowStartRequest( self, unBrowserHandle, bAllowed );
}

void Steam_HTMLSurface_CopyToClipboard( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_CopyToClipboard( self, unBrowserHandle );
}

SteamAPICall_t Steam_HTMLSurface_CreateBrowser( const char * pchUserAgent, const char * pchUserCSS )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTMLSurface_CreateBrowser( self, pchUserAgent, pchUserCSS );
}

void Steam_HTMLSurface_ExecuteJavascript( HHTMLBrowser unBrowserHandle, const char * pchScript )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_ExecuteJavascript( self, unBrowserHandle, pchScript );
}

void Steam_HTMLSurface_Find( HHTMLBrowser unBrowserHandle, const char * pchSearchStr, bool bCurrentlyInFind, bool bReverse )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_Find( self, unBrowserHandle, pchSearchStr, bCurrentlyInFind, bReverse );
}

void Steam_HTMLSurface_GetLinkAtPosition( HHTMLBrowser unBrowserHandle, int x, int y )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_GetLinkAtPosition( self, unBrowserHandle, x, y );
}

void Steam_HTMLSurface_GoBack( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_GoBack( self, unBrowserHandle );
}

void Steam_HTMLSurface_GoForward( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_GoForward( self, unBrowserHandle );
}

bool Steam_HTMLSurface_Init(  )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTMLSurface_Init( self );
}

void Steam_HTMLSurface_JSDialogResponse( HHTMLBrowser unBrowserHandle, bool bResult )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_JSDialogResponse( self, unBrowserHandle, bResult );
}

void Steam_HTMLSurface_LoadURL( HHTMLBrowser unBrowserHandle, const char * pchURL, const char * pchPostData )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_LoadURL( self, unBrowserHandle, pchURL, pchPostData );
}

void Steam_HTMLSurface_MouseMove( HHTMLBrowser unBrowserHandle, int x, int y )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_MouseMove( self, unBrowserHandle, x, y );
}

void Steam_HTMLSurface_MouseWheel( HHTMLBrowser unBrowserHandle, int32 nDelta )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_MouseWheel( self, unBrowserHandle, nDelta );
}

void Steam_HTMLSurface_OpenDeveloperTools( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_OpenDeveloperTools( self, unBrowserHandle );
}

void Steam_HTMLSurface_PasteFromClipboard( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_PasteFromClipboard( self, unBrowserHandle );
}

void Steam_HTMLSurface_Reload( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_Reload( self, unBrowserHandle );
}

void Steam_HTMLSurface_RemoveBrowser( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_RemoveBrowser( self, unBrowserHandle );
}

void Steam_HTMLSurface_SetBackgroundMode( HHTMLBrowser unBrowserHandle, bool bBackgroundMode )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_SetBackgroundMode( self, unBrowserHandle, bBackgroundMode );
}

void Steam_HTMLSurface_SetCookie( const char * pchHostname, const char * pchKey, const char * pchValue, const char * pchPath, RTime32 nExpires, bool bSecure, bool bHTTPOnly )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_SetCookie( self, pchHostname, pchKey, pchValue, pchPath, nExpires, bSecure, bHTTPOnly );
}

void Steam_HTMLSurface_SetDPIScalingFactor( HHTMLBrowser unBrowserHandle, float flDPIScaling )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_SetDPIScalingFactor( self, unBrowserHandle, flDPIScaling );
}

void Steam_HTMLSurface_SetHorizontalScroll( HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_SetHorizontalScroll( self, unBrowserHandle, nAbsolutePixelScroll );
}

void Steam_HTMLSurface_SetKeyFocus( HHTMLBrowser unBrowserHandle, bool bHasKeyFocus )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_SetKeyFocus( self, unBrowserHandle, bHasKeyFocus );
}

void Steam_HTMLSurface_SetPageScaleFactor( HHTMLBrowser unBrowserHandle, float flZoom, int nPointX, int nPointY )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_SetPageScaleFactor( self, unBrowserHandle, flZoom, nPointX, nPointY );
}

void Steam_HTMLSurface_SetSize( HHTMLBrowser unBrowserHandle, uint32 unWidth, uint32 unHeight )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_SetSize( self, unBrowserHandle, unWidth, unHeight );
}

void Steam_HTMLSurface_SetVerticalScroll( HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_SetVerticalScroll( self, unBrowserHandle, nAbsolutePixelScroll );
}

bool Steam_HTMLSurface_Shutdown(  )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTMLSurface_Shutdown( self );
}

void Steam_HTMLSurface_StopFind( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_StopFind( self, unBrowserHandle );
}

void Steam_HTMLSurface_StopLoad( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_StopLoad( self, unBrowserHandle );
}

void Steam_HTMLSurface_ViewSource( HHTMLBrowser unBrowserHandle )
{
	auto *self = SteamAPI_SteamHTMLSurface_v005();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamHTMLSurface_ViewSource( self, unBrowserHandle );
}

HTTPCookieContainerHandle Steam_HTTP_CreateCookieContainer( bool bAllowResponsesToModify )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_CreateCookieContainer( self, bAllowResponsesToModify );
}

HTTPRequestHandle Steam_HTTP_CreateHTTPRequest( EHTTPMethod eHTTPRequestMethod, const char * pchAbsoluteURL )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_CreateHTTPRequest( self, eHTTPRequestMethod, pchAbsoluteURL );
}

bool Steam_HTTP_DeferHTTPRequest( HTTPRequestHandle hRequest )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_DeferHTTPRequest( self, hRequest );
}

bool Steam_HTTP_PrioritizeHTTPRequest( HTTPRequestHandle hRequest )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_PrioritizeHTTPRequest( self, hRequest );
}

bool Steam_HTTP_ReleaseCookieContainer( HTTPCookieContainerHandle hCookieContainer )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_ReleaseCookieContainer( self, hCookieContainer );
}

bool Steam_HTTP_ReleaseHTTPRequest( HTTPRequestHandle hRequest )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_ReleaseHTTPRequest( self, hRequest );
}

bool Steam_HTTP_SetCookie( HTTPCookieContainerHandle hCookieContainer, const char * pchHost, const char * pchUrl, const char * pchCookie )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetCookie( self, hCookieContainer, pchHost, pchUrl, pchCookie );
}

bool Steam_HTTP_SetHTTPRequestAbsoluteTimeoutMS( HTTPRequestHandle hRequest, uint32 unMilliseconds )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetHTTPRequestAbsoluteTimeoutMS( self, hRequest, unMilliseconds );
}

bool Steam_HTTP_SetHTTPRequestContextValue( HTTPRequestHandle hRequest, uint64 ulContextValue )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetHTTPRequestContextValue( self, hRequest, ulContextValue );
}

bool Steam_HTTP_SetHTTPRequestCookieContainer( HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetHTTPRequestCookieContainer( self, hRequest, hCookieContainer );
}

bool Steam_HTTP_SetHTTPRequestGetOrPostParameter( HTTPRequestHandle hRequest, const char * pchParamName, const char * pchParamValue )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetHTTPRequestGetOrPostParameter( self, hRequest, pchParamName, pchParamValue );
}

bool Steam_HTTP_SetHTTPRequestHeaderValue( HTTPRequestHandle hRequest, const char * pchHeaderName, const char * pchHeaderValue )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetHTTPRequestHeaderValue( self, hRequest, pchHeaderName, pchHeaderValue );
}

bool Steam_HTTP_SetHTTPRequestNetworkActivityTimeout( HTTPRequestHandle hRequest, uint32 unTimeoutSeconds )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetHTTPRequestNetworkActivityTimeout( self, hRequest, unTimeoutSeconds );
}

bool Steam_HTTP_SetHTTPRequestRequiresVerifiedCertificate( HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetHTTPRequestRequiresVerifiedCertificate( self, hRequest, bRequireVerifiedCertificate );
}

bool Steam_HTTP_SetHTTPRequestUserAgentInfo( HTTPRequestHandle hRequest, const char * pchUserAgentInfo )
{
	auto *self = SteamAPI_SteamHTTP_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamHTTP_SetHTTPRequestUserAgentInfo( self, hRequest, pchUserAgentInfo );
}

void Steam_Input_ActivateActionSet( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_ActivateActionSet( self, inputHandle, actionSetHandle );
}

void Steam_Input_ActivateActionSetLayer( InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_ActivateActionSetLayer( self, inputHandle, actionSetLayerHandle );
}

bool Steam_Input_BNewDataAvailable(  )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_BNewDataAvailable( self );
}

bool Steam_Input_BWaitForData( bool bWaitForever, uint32 unTimeout )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_BWaitForData( self, bWaitForever, unTimeout );
}

void Steam_Input_DeactivateActionSetLayer( InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_DeactivateActionSetLayer( self, inputHandle, actionSetLayerHandle );
}

void Steam_Input_DeactivateAllActionSetLayers( InputHandle_t inputHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_DeactivateAllActionSetLayers( self, inputHandle );
}

void Steam_Input_EnableActionEventCallbacks( SteamInputActionEventCallbackPointer pCallback )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_EnableActionEventCallbacks( self, pCallback );
}

void Steam_Input_EnableDeviceCallbacks(  )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_EnableDeviceCallbacks( self );
}

EInputActionOrigin Steam_Input_GetActionOriginFromXboxOrigin( InputHandle_t inputHandle, EXboxOrigin eOrigin )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetActionOriginFromXboxOrigin( self, inputHandle, eOrigin );
}

InputActionSetHandle_t Steam_Input_GetActionSetHandle( const char * pszActionSetName )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetActionSetHandle( self, pszActionSetName );
}

InputAnalogActionData_t Steam_Input_GetAnalogActionData( InputHandle_t inputHandle, InputAnalogActionHandle_t analogActionHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetAnalogActionData( self, inputHandle, analogActionHandle );
}

InputAnalogActionHandle_t Steam_Input_GetAnalogActionHandle( const char * pszActionName )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetAnalogActionHandle( self, pszActionName );
}

InputHandle_t Steam_Input_GetControllerForGamepadIndex( int nIndex )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetControllerForGamepadIndex( self, nIndex );
}

InputActionSetHandle_t Steam_Input_GetCurrentActionSet( InputHandle_t inputHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetCurrentActionSet( self, inputHandle );
}

InputDigitalActionData_t Steam_Input_GetDigitalActionData( InputHandle_t inputHandle, InputDigitalActionHandle_t digitalActionHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetDigitalActionData( self, inputHandle, digitalActionHandle );
}

InputDigitalActionHandle_t Steam_Input_GetDigitalActionHandle( const char * pszActionName )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetDigitalActionHandle( self, pszActionName );
}

int Steam_Input_GetGamepadIndexForController( InputHandle_t ulinputHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetGamepadIndexForController( self, ulinputHandle );
}

const char * Steam_Input_GetGlyphForActionOrigin_Legacy( EInputActionOrigin eOrigin )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetGlyphForActionOrigin_Legacy( self, eOrigin );
}

const char * Steam_Input_GetGlyphForXboxOrigin( EXboxOrigin eOrigin )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetGlyphForXboxOrigin( self, eOrigin );
}

const char * Steam_Input_GetGlyphPNGForActionOrigin( EInputActionOrigin eOrigin, ESteamInputGlyphSize eSize, uint32 unFlags )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetGlyphPNGForActionOrigin( self, eOrigin, eSize, unFlags );
}

const char * Steam_Input_GetGlyphSVGForActionOrigin( EInputActionOrigin eOrigin, uint32 unFlags )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetGlyphSVGForActionOrigin( self, eOrigin, unFlags );
}

ESteamInputType Steam_Input_GetInputTypeForHandle( InputHandle_t inputHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetInputTypeForHandle( self, inputHandle );
}

InputMotionData_t Steam_Input_GetMotionData( InputHandle_t inputHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetMotionData( self, inputHandle );
}

uint32 Steam_Input_GetRemotePlaySessionID( InputHandle_t inputHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetRemotePlaySessionID( self, inputHandle );
}

uint16 Steam_Input_GetSessionInputConfigurationSettings(  )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetSessionInputConfigurationSettings( self );
}

const char * Steam_Input_GetStringForActionOrigin( EInputActionOrigin eOrigin )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetStringForActionOrigin( self, eOrigin );
}

const char * Steam_Input_GetStringForAnalogActionName( InputAnalogActionHandle_t eActionHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetStringForAnalogActionName( self, eActionHandle );
}

const char * Steam_Input_GetStringForDigitalActionName( InputDigitalActionHandle_t eActionHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetStringForDigitalActionName( self, eActionHandle );
}

const char * Steam_Input_GetStringForXboxOrigin( EXboxOrigin eOrigin )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_GetStringForXboxOrigin( self, eOrigin );
}

bool Steam_Input_Init( bool bExplicitlyCallRunFrame )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_Init( self, bExplicitlyCallRunFrame );
}

void Steam_Input_Legacy_TriggerHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_Legacy_TriggerHapticPulse( self, inputHandle, eTargetPad, usDurationMicroSec );
}

void Steam_Input_Legacy_TriggerRepeatedHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_Legacy_TriggerRepeatedHapticPulse( self, inputHandle, eTargetPad, usDurationMicroSec, usOffMicroSec, unRepeat, nFlags );
}

void Steam_Input_RunFrame( bool bReservedValue )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_RunFrame( self, bReservedValue );
}

bool Steam_Input_SetInputActionManifestFilePath( const char * pchInputActionManifestAbsolutePath )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_SetInputActionManifestFilePath( self, pchInputActionManifestAbsolutePath );
}

void Steam_Input_SetLEDColor( InputHandle_t inputHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_SetLEDColor( self, inputHandle, nColorR, nColorG, nColorB, nFlags );
}

bool Steam_Input_ShowBindingPanel( InputHandle_t inputHandle )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_ShowBindingPanel( self, inputHandle );
}

bool Steam_Input_Shutdown(  )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_Shutdown( self );
}

void Steam_Input_StopAnalogActionMomentum( InputHandle_t inputHandle, InputAnalogActionHandle_t eAction )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_StopAnalogActionMomentum( self, inputHandle, eAction );
}

EInputActionOrigin Steam_Input_TranslateActionOrigin( ESteamInputType eDestinationInputType, EInputActionOrigin eSourceOrigin )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInput_TranslateActionOrigin( self, eDestinationInputType, eSourceOrigin );
}

void Steam_Input_TriggerSimpleHapticEvent( InputHandle_t inputHandle, EControllerHapticLocation eHapticLocation, uint8 nIntensity, char nGainDB, uint8 nOtherIntensity, char nOtherGainDB )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_TriggerSimpleHapticEvent( self, inputHandle, eHapticLocation, nIntensity, nGainDB, nOtherIntensity, nOtherGainDB );
}

void Steam_Input_TriggerVibration( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_TriggerVibration( self, inputHandle, usLeftSpeed, usRightSpeed );
}

void Steam_Input_TriggerVibrationExtended( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed, unsigned short usLeftTriggerSpeed, unsigned short usRightTriggerSpeed )
{
	auto *self = SteamAPI_SteamInput_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInput_TriggerVibrationExtended( self, inputHandle, usLeftSpeed, usRightSpeed, usLeftTriggerSpeed, usRightTriggerSpeed );
}

bool Steam_Inventory_CheckResultSteamID( SteamInventoryResult_t resultHandle, uint64_steamid steamIDExpected )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_CheckResultSteamID( self, resultHandle, steamIDExpected );
}

void Steam_Inventory_DestroyResult( SteamInventoryResult_t resultHandle )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInventory_DestroyResult( self, resultHandle );
}

uint32 Steam_Inventory_GetNumItemsWithPrices(  )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_GetNumItemsWithPrices( self );
}

EResult Steam_Inventory_GetResultStatus( SteamInventoryResult_t resultHandle )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_GetResultStatus( self, resultHandle );
}

uint32 Steam_Inventory_GetResultTimestamp( SteamInventoryResult_t resultHandle )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_GetResultTimestamp( self, resultHandle );
}

bool Steam_Inventory_LoadItemDefinitions(  )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_LoadItemDefinitions( self );
}

bool Steam_Inventory_RemoveProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_RemoveProperty( self, handle, nItemID, pchPropertyName );
}

SteamAPICall_t Steam_Inventory_RequestEligiblePromoItemDefinitionsIDs( uint64_steamid steamID )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_RequestEligiblePromoItemDefinitionsIDs( self, steamID );
}

SteamAPICall_t Steam_Inventory_RequestPrices(  )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_RequestPrices( self );
}

void Steam_Inventory_SendItemDropHeartbeat(  )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamInventory_SendItemDropHeartbeat( self );
}

bool Steam_Inventory_SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, const char * pchPropertyValue )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_SetPropertyString( self, handle, nItemID, pchPropertyName, pchPropertyValue );
}

bool Steam_Inventory_SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, bool bValue )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_SetPropertyBool( self, handle, nItemID, pchPropertyName, bValue );
}

bool Steam_Inventory_SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, int64 nValue )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_SetPropertyInt64( self, handle, nItemID, pchPropertyName, nValue );
}

bool Steam_Inventory_SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, float flValue )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_SetPropertyFloat( self, handle, nItemID, pchPropertyName, flValue );
}

SteamInventoryUpdateHandle_t Steam_Inventory_StartUpdateProperties(  )
{
	auto *self = SteamAPI_SteamInventory_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamInventory_StartUpdateProperties( self );
}

void Steam_MatchmakingServers_CancelQuery( HServerListRequest hRequest )
{
	auto *self = SteamAPI_SteamMatchmakingServers_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmakingServers_CancelQuery( self, hRequest );
}

void Steam_MatchmakingServers_CancelServerQuery( HServerQuery hServerQuery )
{
	auto *self = SteamAPI_SteamMatchmakingServers_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmakingServers_CancelServerQuery( self, hServerQuery );
}

int Steam_MatchmakingServers_GetServerCount( HServerListRequest hRequest )
{
	auto *self = SteamAPI_SteamMatchmakingServers_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmakingServers_GetServerCount( self, hRequest );
}

bool Steam_MatchmakingServers_IsRefreshing( HServerListRequest hRequest )
{
	auto *self = SteamAPI_SteamMatchmakingServers_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmakingServers_IsRefreshing( self, hRequest );
}

void Steam_MatchmakingServers_RefreshQuery( HServerListRequest hRequest )
{
	auto *self = SteamAPI_SteamMatchmakingServers_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmakingServers_RefreshQuery( self, hRequest );
}

void Steam_MatchmakingServers_RefreshServer( HServerListRequest hRequest, int iServer )
{
	auto *self = SteamAPI_SteamMatchmakingServers_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmakingServers_RefreshServer( self, hRequest, iServer );
}

void Steam_MatchmakingServers_ReleaseRequest( HServerListRequest hServerListRequest )
{
	auto *self = SteamAPI_SteamMatchmakingServers_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmakingServers_ReleaseRequest( self, hServerListRequest );
}

int Steam_Matchmaking_AddFavoriteGame( AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_AddFavoriteGame( self, nAppID, nIP, nConnPort, nQueryPort, unFlags, rTime32LastPlayedOnServer );
}

void Steam_Matchmaking_AddRequestLobbyListCompatibleMembersFilter( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_AddRequestLobbyListCompatibleMembersFilter( self, steamIDLobby );
}

void Steam_Matchmaking_AddRequestLobbyListDistanceFilter( ELobbyDistanceFilter eLobbyDistanceFilter )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter( self, eLobbyDistanceFilter );
}

void Steam_Matchmaking_AddRequestLobbyListFilterSlotsAvailable( int nSlotsAvailable )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable( self, nSlotsAvailable );
}

void Steam_Matchmaking_AddRequestLobbyListNearValueFilter( const char * pchKeyToMatch, int nValueToBeCloseTo )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter( self, pchKeyToMatch, nValueToBeCloseTo );
}

void Steam_Matchmaking_AddRequestLobbyListNumericalFilter( const char * pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter( self, pchKeyToMatch, nValueToMatch, eComparisonType );
}

void Steam_Matchmaking_AddRequestLobbyListResultCountFilter( int cMaxResults )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter( self, cMaxResults );
}

void Steam_Matchmaking_AddRequestLobbyListStringFilter( const char * pchKeyToMatch, const char * pchValueToMatch, ELobbyComparison eComparisonType )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter( self, pchKeyToMatch, pchValueToMatch, eComparisonType );
}

SteamAPICall_t Steam_Matchmaking_CreateLobby( ELobbyType eLobbyType, int cMaxMembers )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_CreateLobby( self, eLobbyType, cMaxMembers );
}

bool Steam_Matchmaking_DeleteLobbyData( uint64_steamid steamIDLobby, const char * pchKey )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_DeleteLobbyData( self, steamIDLobby, pchKey );
}

int Steam_Matchmaking_GetFavoriteGameCount(  )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetFavoriteGameCount( self );
}

uint64_steamid Steam_Matchmaking_GetLobbyByIndex( int iLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetLobbyByIndex( self, iLobby );
}

const char * Steam_Matchmaking_GetLobbyData( uint64_steamid steamIDLobby, const char * pchKey )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetLobbyData( self, steamIDLobby, pchKey );
}

int Steam_Matchmaking_GetLobbyDataCount( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetLobbyDataCount( self, steamIDLobby );
}

uint64_steamid Steam_Matchmaking_GetLobbyMemberByIndex( uint64_steamid steamIDLobby, int iMember )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex( self, steamIDLobby, iMember );
}

const char * Steam_Matchmaking_GetLobbyMemberData( uint64_steamid steamIDLobby, uint64_steamid steamIDUser, const char * pchKey )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetLobbyMemberData( self, steamIDLobby, steamIDUser, pchKey );
}

int Steam_Matchmaking_GetLobbyMemberLimit( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetLobbyMemberLimit( self, steamIDLobby );
}

uint64_steamid Steam_Matchmaking_GetLobbyOwner( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetLobbyOwner( self, steamIDLobby );
}

int Steam_Matchmaking_GetNumLobbyMembers( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_GetNumLobbyMembers( self, steamIDLobby );
}

bool Steam_Matchmaking_InviteUserToLobby( uint64_steamid steamIDLobby, uint64_steamid steamIDInvitee )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_InviteUserToLobby( self, steamIDLobby, steamIDInvitee );
}

SteamAPICall_t Steam_Matchmaking_JoinLobby( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_JoinLobby( self, steamIDLobby );
}

void Steam_Matchmaking_LeaveLobby( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_LeaveLobby( self, steamIDLobby );
}

bool Steam_Matchmaking_RemoveFavoriteGame( AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_RemoveFavoriteGame( self, nAppID, nIP, nConnPort, nQueryPort, unFlags );
}

bool Steam_Matchmaking_RequestLobbyData( uint64_steamid steamIDLobby )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_RequestLobbyData( self, steamIDLobby );
}

SteamAPICall_t Steam_Matchmaking_RequestLobbyList(  )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_RequestLobbyList( self );
}

bool Steam_Matchmaking_SetLinkedLobby( uint64_steamid steamIDLobby, uint64_steamid steamIDLobbyDependent )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_SetLinkedLobby( self, steamIDLobby, steamIDLobbyDependent );
}

bool Steam_Matchmaking_SetLobbyData( uint64_steamid steamIDLobby, const char * pchKey, const char * pchValue )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_SetLobbyData( self, steamIDLobby, pchKey, pchValue );
}

void Steam_Matchmaking_SetLobbyGameServer( uint64_steamid steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, uint64_steamid steamIDGameServer )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_SetLobbyGameServer( self, steamIDLobby, unGameServerIP, unGameServerPort, steamIDGameServer );
}

bool Steam_Matchmaking_SetLobbyJoinable( uint64_steamid steamIDLobby, bool bLobbyJoinable )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_SetLobbyJoinable( self, steamIDLobby, bLobbyJoinable );
}

void Steam_Matchmaking_SetLobbyMemberData( uint64_steamid steamIDLobby, const char * pchKey, const char * pchValue )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMatchmaking_SetLobbyMemberData( self, steamIDLobby, pchKey, pchValue );
}

bool Steam_Matchmaking_SetLobbyMemberLimit( uint64_steamid steamIDLobby, int cMaxMembers )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_SetLobbyMemberLimit( self, steamIDLobby, cMaxMembers );
}

bool Steam_Matchmaking_SetLobbyOwner( uint64_steamid steamIDLobby, uint64_steamid steamIDNewOwner )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_SetLobbyOwner( self, steamIDLobby, steamIDNewOwner );
}

bool Steam_Matchmaking_SetLobbyType( uint64_steamid steamIDLobby, ELobbyType eLobbyType )
{
	auto *self = SteamAPI_SteamMatchmaking_v009();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMatchmaking_SetLobbyType( self, steamIDLobby, eLobbyType );
}

bool Steam_Music_BIsEnabled(  )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMusic_BIsEnabled( self );
}

bool Steam_Music_BIsPlaying(  )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMusic_BIsPlaying( self );
}

AudioPlayback_Status Steam_Music_GetPlaybackStatus(  )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMusic_GetPlaybackStatus( self );
}

float Steam_Music_GetVolume(  )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamMusic_GetVolume( self );
}

void Steam_Music_Pause(  )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMusic_Pause( self );
}

void Steam_Music_Play(  )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMusic_Play( self );
}

void Steam_Music_PlayNext(  )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMusic_PlayNext( self );
}

void Steam_Music_PlayPrevious(  )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMusic_PlayPrevious( self );
}

void Steam_Music_SetVolume( float flVolume )
{
	auto *self = SteamAPI_SteamMusic_v001();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamMusic_SetVolume( self, flVolume );
}

EResult Steam_NetworkingSockets_AcceptConnection( HSteamNetConnection hConn )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_AcceptConnection( self, hConn );
}

bool Steam_NetworkingSockets_BeginAsyncRequestFakeIP( int nNumPorts )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_BeginAsyncRequestFakeIP( self, nNumPorts );
}

bool Steam_NetworkingSockets_CloseConnection( HSteamNetConnection hPeer, int nReason, const char * pszDebug, bool bEnableLinger )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_CloseConnection( self, hPeer, nReason, pszDebug, bEnableLinger );
}

bool Steam_NetworkingSockets_CloseListenSocket( HSteamListenSocket hSocket )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_CloseListenSocket( self, hSocket );
}

HSteamNetPollGroup Steam_NetworkingSockets_CreatePollGroup(  )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_CreatePollGroup( self );
}

bool Steam_NetworkingSockets_DestroyPollGroup( HSteamNetPollGroup hPollGroup )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_DestroyPollGroup( self, hPollGroup );
}

EResult Steam_NetworkingSockets_FlushMessagesOnConnection( HSteamNetConnection hConn )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_FlushMessagesOnConnection( self, hConn );
}

int64 Steam_NetworkingSockets_GetConnectionUserData( HSteamNetConnection hPeer )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_GetConnectionUserData( self, hPeer );
}

SteamNetworkingPOPID Steam_NetworkingSockets_GetHostedDedicatedServerPOPID(  )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPOPID( self );
}

uint16 Steam_NetworkingSockets_GetHostedDedicatedServerPort(  )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPort( self );
}

ESteamNetworkingAvailability Steam_NetworkingSockets_InitAuthentication(  )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_InitAuthentication( self );
}

void Steam_NetworkingSockets_RunCallbacks(  )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamNetworkingSockets_RunCallbacks( self );
}

void Steam_NetworkingSockets_SetConnectionName( HSteamNetConnection hPeer, const char * pszName )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamNetworkingSockets_SetConnectionName( self, hPeer, pszName );
}

bool Steam_NetworkingSockets_SetConnectionPollGroup( HSteamNetConnection hConn, HSteamNetPollGroup hPollGroup )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_SetConnectionPollGroup( self, hConn, hPollGroup );
}

bool Steam_NetworkingSockets_SetConnectionUserData( HSteamNetConnection hPeer, int64 nUserData )
{
	auto *self = SteamAPI_SteamNetworkingSockets_SteamAPI_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingSockets_SetConnectionUserData( self, hPeer, nUserData );
}

bool Steam_NetworkingUtils_CheckPingDataUpToDate( float flMaxAgeSeconds )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_CheckPingDataUpToDate( self, flMaxAgeSeconds );
}

int Steam_NetworkingUtils_GetDirectPingToPOP( SteamNetworkingPOPID popID )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_GetDirectPingToPOP( self, popID );
}

ESteamNetworkingFakeIPType Steam_NetworkingUtils_GetIPv4FakeIPType( uint32 nIPv4 )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_GetIPv4FakeIPType( self, nIPv4 );
}

SteamNetworkingMicroseconds Steam_NetworkingUtils_GetLocalTimestamp(  )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_GetLocalTimestamp( self );
}

int Steam_NetworkingUtils_GetPOPCount(  )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_GetPOPCount( self );
}

void Steam_NetworkingUtils_InitRelayNetworkAccess(  )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess( self );
}

bool Steam_NetworkingUtils_IsFakeIPv4( uint32 nIPv4 )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_IsFakeIPv4( self, nIPv4 );
}

ESteamNetworkingConfigValue Steam_NetworkingUtils_IterateGenericEditableConfigValues( ESteamNetworkingConfigValue eCurrent, bool bEnumerateDevVars )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_IterateGenericEditableConfigValues( self, eCurrent, bEnumerateDevVars );
}

bool Steam_NetworkingUtils_SetConnectionConfigValueFloat( HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, float val )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueFloat( self, hConn, eValue, val );
}

bool Steam_NetworkingUtils_SetConnectionConfigValueInt32( HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, int32 val )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueInt32( self, hConn, eValue, val );
}

bool Steam_NetworkingUtils_SetConnectionConfigValueString( HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, const char * val )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueString( self, hConn, eValue, val );
}

void Steam_NetworkingUtils_SetDebugOutputFunction( ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamNetworkingUtils_SetDebugOutputFunction( self, eDetailLevel, pfnFunc );
}

bool Steam_NetworkingUtils_SetGlobalCallback_FakeIPResult( FnSteamNetworkingFakeIPResult fnCallback )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_FakeIPResult( self, fnCallback );
}

bool Steam_NetworkingUtils_SetGlobalCallback_MessagesSessionFailed( FnSteamNetworkingMessagesSessionFailed fnCallback )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_MessagesSessionFailed( self, fnCallback );
}

bool Steam_NetworkingUtils_SetGlobalCallback_MessagesSessionRequest( FnSteamNetworkingMessagesSessionRequest fnCallback )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_MessagesSessionRequest( self, fnCallback );
}

bool Steam_NetworkingUtils_SetGlobalCallback_SteamNetAuthenticationStatusChanged( FnSteamNetAuthenticationStatusChanged fnCallback )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamNetAuthenticationStatusChanged( self, fnCallback );
}

bool Steam_NetworkingUtils_SetGlobalCallback_SteamNetConnectionStatusChanged( FnSteamNetConnectionStatusChanged fnCallback )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamNetConnectionStatusChanged( self, fnCallback );
}

bool Steam_NetworkingUtils_SetGlobalCallback_SteamRelayNetworkStatusChanged( FnSteamRelayNetworkStatusChanged fnCallback )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamRelayNetworkStatusChanged( self, fnCallback );
}

bool Steam_NetworkingUtils_SetGlobalConfigValueFloat( ESteamNetworkingConfigValue eValue, float val )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueFloat( self, eValue, val );
}

bool Steam_NetworkingUtils_SetGlobalConfigValueInt32( ESteamNetworkingConfigValue eValue, int32 val )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueInt32( self, eValue, val );
}

bool Steam_NetworkingUtils_SetGlobalConfigValueString( ESteamNetworkingConfigValue eValue, const char * val )
{
	auto *self = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueString( self, eValue, val );
}

bool Steam_Networking_AcceptP2PSessionWithUser( uint64_steamid steamIDRemote )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser( self, steamIDRemote );
}

bool Steam_Networking_AllowP2PPacketRelay( bool bAllow )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_AllowP2PPacketRelay( self, bAllow );
}

bool Steam_Networking_CloseP2PChannelWithUser( uint64_steamid steamIDRemote, int nChannel )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_CloseP2PChannelWithUser( self, steamIDRemote, nChannel );
}

bool Steam_Networking_CloseP2PSessionWithUser( uint64_steamid steamIDRemote )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_CloseP2PSessionWithUser( self, steamIDRemote );
}

SNetSocket_t Steam_Networking_CreateConnectionSocket( SteamIPAddress_t nIP, uint16 nPort, int nTimeoutSec )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_CreateConnectionSocket( self, nIP, nPort, nTimeoutSec );
}

SNetListenSocket_t Steam_Networking_CreateListenSocket( int nVirtualP2PPort, SteamIPAddress_t nIP, uint16 nPort, bool bAllowUseOfPacketRelay )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_CreateListenSocket( self, nVirtualP2PPort, nIP, nPort, bAllowUseOfPacketRelay );
}

SNetSocket_t Steam_Networking_CreateP2PConnectionSocket( uint64_steamid steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_CreateP2PConnectionSocket( self, steamIDTarget, nVirtualPort, nTimeoutSec, bAllowUseOfPacketRelay );
}

bool Steam_Networking_DestroyListenSocket( SNetListenSocket_t hSocket, bool bNotifyRemoteEnd )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_DestroyListenSocket( self, hSocket, bNotifyRemoteEnd );
}

bool Steam_Networking_DestroySocket( SNetSocket_t hSocket, bool bNotifyRemoteEnd )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_DestroySocket( self, hSocket, bNotifyRemoteEnd );
}

int Steam_Networking_GetMaxPacketSize( SNetSocket_t hSocket )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_GetMaxPacketSize( self, hSocket );
}

ESNetSocketConnectionType Steam_Networking_GetSocketConnectionType( SNetSocket_t hSocket )
{
	auto *self = SteamAPI_SteamNetworking_v006();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamNetworking_GetSocketConnectionType( self, hSocket );
}

bool Steam_ParentalSettings_BIsAppBlocked( AppId_t nAppID )
{
	auto *self = SteamAPI_SteamParentalSettings_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParentalSettings_BIsAppBlocked( self, nAppID );
}

bool Steam_ParentalSettings_BIsAppInBlockList( AppId_t nAppID )
{
	auto *self = SteamAPI_SteamParentalSettings_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParentalSettings_BIsAppInBlockList( self, nAppID );
}

bool Steam_ParentalSettings_BIsFeatureBlocked( EParentalFeature eFeature )
{
	auto *self = SteamAPI_SteamParentalSettings_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParentalSettings_BIsFeatureBlocked( self, eFeature );
}

bool Steam_ParentalSettings_BIsFeatureInBlockList( EParentalFeature eFeature )
{
	auto *self = SteamAPI_SteamParentalSettings_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParentalSettings_BIsFeatureInBlockList( self, eFeature );
}

bool Steam_ParentalSettings_BIsParentalLockEnabled(  )
{
	auto *self = SteamAPI_SteamParentalSettings_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParentalSettings_BIsParentalLockEnabled( self );
}

bool Steam_ParentalSettings_BIsParentalLockLocked(  )
{
	auto *self = SteamAPI_SteamParentalSettings_v001();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParentalSettings_BIsParentalLockLocked( self );
}

void Steam_Parties_CancelReservation( PartyBeaconID_t ulBeacon, uint64_steamid steamIDUser )
{
	auto *self = SteamAPI_SteamParties_v002();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamParties_CancelReservation( self, ulBeacon, steamIDUser );
}

SteamAPICall_t Steam_Parties_ChangeNumOpenSlots( PartyBeaconID_t ulBeacon, uint32 unOpenSlots )
{
	auto *self = SteamAPI_SteamParties_v002();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParties_ChangeNumOpenSlots( self, ulBeacon, unOpenSlots );
}

bool Steam_Parties_DestroyBeacon( PartyBeaconID_t ulBeacon )
{
	auto *self = SteamAPI_SteamParties_v002();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParties_DestroyBeacon( self, ulBeacon );
}

PartyBeaconID_t Steam_Parties_GetBeaconByIndex( uint32 unIndex )
{
	auto *self = SteamAPI_SteamParties_v002();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParties_GetBeaconByIndex( self, unIndex );
}

uint32 Steam_Parties_GetNumActiveBeacons(  )
{
	auto *self = SteamAPI_SteamParties_v002();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParties_GetNumActiveBeacons( self );
}

SteamAPICall_t Steam_Parties_JoinParty( PartyBeaconID_t ulBeaconID )
{
	auto *self = SteamAPI_SteamParties_v002();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamParties_JoinParty( self, ulBeaconID );
}

void Steam_Parties_OnReservationCompleted( PartyBeaconID_t ulBeacon, uint64_steamid steamIDUser )
{
	auto *self = SteamAPI_SteamParties_v002();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamParties_OnReservationCompleted( self, ulBeacon, steamIDUser );
}

bool Steam_RemotePlay_BEnableRemotePlayTogetherDirectInput(  )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_BEnableRemotePlayTogetherDirectInput( self );
}

bool Steam_RemotePlay_BSendRemotePlayTogetherInvite( uint64_steamid steamIDFriend )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_BSendRemotePlayTogetherInvite( self, steamIDFriend );
}

bool Steam_RemotePlay_BSessionRemotePlayTogether( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_BSessionRemotePlayTogether( self, unSessionID );
}

void Steam_RemotePlay_DisableRemotePlayTogetherDirectInput(  )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamRemotePlay_DisableRemotePlayTogetherDirectInput( self );
}

int Steam_RemotePlay_GetLargeSessionAvatar( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetLargeSessionAvatar( self, unSessionID );
}

int Steam_RemotePlay_GetMediumSessionAvatar( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetMediumSessionAvatar( self, unSessionID );
}

ESteamDeviceFormFactor Steam_RemotePlay_GetSessionClientFormFactor( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetSessionClientFormFactor( self, unSessionID );
}

const char * Steam_RemotePlay_GetSessionClientName( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetSessionClientName( self, unSessionID );
}

uint32 Steam_RemotePlay_GetSessionCount(  )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetSessionCount( self );
}

uint32 Steam_RemotePlay_GetSessionGuestID( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetSessionGuestID( self, unSessionID );
}

RemotePlaySessionID_t Steam_RemotePlay_GetSessionID( int iSessionIndex )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetSessionID( self, iSessionIndex );
}

uint64_steamid Steam_RemotePlay_GetSessionSteamID( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetSessionSteamID( self, unSessionID );
}

int Steam_RemotePlay_GetSmallSessionAvatar( RemotePlaySessionID_t unSessionID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_GetSmallSessionAvatar( self, unSessionID );
}

void Steam_RemotePlay_SetMouseCursor( RemotePlaySessionID_t unSessionID, RemotePlayCursorID_t unCursorID )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamRemotePlay_SetMouseCursor( self, unSessionID, unCursorID );
}

void Steam_RemotePlay_SetMousePosition( RemotePlaySessionID_t unSessionID, float flNormalizedX, float flNormalizedY )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamRemotePlay_SetMousePosition( self, unSessionID, flNormalizedX, flNormalizedY );
}

void Steam_RemotePlay_SetMouseVisibility( RemotePlaySessionID_t unSessionID, bool bVisible )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamRemotePlay_SetMouseVisibility( self, unSessionID, bVisible );
}

bool Steam_RemotePlay_ShowRemotePlayTogetherUI(  )
{
	auto *self = SteamAPI_SteamRemotePlay_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemotePlay_ShowRemotePlayTogetherUI( self );
}

bool Steam_RemoteStorage_BeginFileWriteBatch(  )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_BeginFileWriteBatch( self );
}

SteamAPICall_t Steam_RemoteStorage_CommitPublishedFileUpdate( PublishedFileUpdateHandle_t updateHandle )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_CommitPublishedFileUpdate( self, updateHandle );
}

PublishedFileUpdateHandle_t Steam_RemoteStorage_CreatePublishedFileUpdateRequest( PublishedFileId_t unPublishedFileId )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_CreatePublishedFileUpdateRequest( self, unPublishedFileId );
}

SteamAPICall_t Steam_RemoteStorage_DeletePublishedFile( PublishedFileId_t unPublishedFileId )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_DeletePublishedFile( self, unPublishedFileId );
}

bool Steam_RemoteStorage_EndFileWriteBatch(  )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_EndFileWriteBatch( self );
}

SteamAPICall_t Steam_RemoteStorage_EnumeratePublishedFilesByUserAction( EWorkshopFileAction eAction, uint32 unStartIndex )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_EnumeratePublishedFilesByUserAction( self, eAction, unStartIndex );
}

SteamAPICall_t Steam_RemoteStorage_EnumerateUserPublishedFiles( uint32 unStartIndex )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_EnumerateUserPublishedFiles( self, unStartIndex );
}

SteamAPICall_t Steam_RemoteStorage_EnumerateUserSubscribedFiles( uint32 unStartIndex )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_EnumerateUserSubscribedFiles( self, unStartIndex );
}

bool Steam_RemoteStorage_FileDelete( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FileDelete( self, pchFile );
}

bool Steam_RemoteStorage_FileExists( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FileExists( self, pchFile );
}

bool Steam_RemoteStorage_FileForget( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FileForget( self, pchFile );
}

bool Steam_RemoteStorage_FilePersisted( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FilePersisted( self, pchFile );
}

SteamAPICall_t Steam_RemoteStorage_FileReadAsync( const char * pchFile, uint32 nOffset, uint32 cubToRead )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FileReadAsync( self, pchFile, nOffset, cubToRead );
}

SteamAPICall_t Steam_RemoteStorage_FileShare( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FileShare( self, pchFile );
}

bool Steam_RemoteStorage_FileWriteStreamCancel( UGCFileWriteStreamHandle_t writeHandle )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FileWriteStreamCancel( self, writeHandle );
}

bool Steam_RemoteStorage_FileWriteStreamClose( UGCFileWriteStreamHandle_t writeHandle )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FileWriteStreamClose( self, writeHandle );
}

UGCFileWriteStreamHandle_t Steam_RemoteStorage_FileWriteStreamOpen( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_FileWriteStreamOpen( self, pchFile );
}

int32 Steam_RemoteStorage_GetCachedUGCCount(  )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetCachedUGCCount( self );
}

UGCHandle_t Steam_RemoteStorage_GetCachedUGCHandle( int32 iCachedContent )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetCachedUGCHandle( self, iCachedContent );
}

int32 Steam_RemoteStorage_GetFileCount(  )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetFileCount( self );
}

int32 Steam_RemoteStorage_GetFileSize( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetFileSize( self, pchFile );
}

int64 Steam_RemoteStorage_GetFileTimestamp( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetFileTimestamp( self, pchFile );
}

int32 Steam_RemoteStorage_GetLocalFileChangeCount(  )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetLocalFileChangeCount( self );
}

SteamAPICall_t Steam_RemoteStorage_GetPublishedFileDetails( PublishedFileId_t unPublishedFileId, uint32 unMaxSecondsOld )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetPublishedFileDetails( self, unPublishedFileId, unMaxSecondsOld );
}

SteamAPICall_t Steam_RemoteStorage_GetPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetPublishedItemVoteDetails( self, unPublishedFileId );
}

ERemoteStoragePlatform Steam_RemoteStorage_GetSyncPlatforms( const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetSyncPlatforms( self, pchFile );
}

SteamAPICall_t Steam_RemoteStorage_GetUserPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_GetUserPublishedItemVoteDetails( self, unPublishedFileId );
}

bool Steam_RemoteStorage_IsCloudEnabledForAccount(  )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_IsCloudEnabledForAccount( self );
}

bool Steam_RemoteStorage_IsCloudEnabledForApp(  )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_IsCloudEnabledForApp( self );
}

void Steam_RemoteStorage_SetCloudEnabledForApp( bool bEnabled )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamRemoteStorage_SetCloudEnabledForApp( self, bEnabled );
}

bool Steam_RemoteStorage_SetSyncPlatforms( const char * pchFile, ERemoteStoragePlatform eRemoteStoragePlatform )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_SetSyncPlatforms( self, pchFile, eRemoteStoragePlatform );
}

SteamAPICall_t Steam_RemoteStorage_SetUserPublishedFileAction( PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_SetUserPublishedFileAction( self, unPublishedFileId, eAction );
}

SteamAPICall_t Steam_RemoteStorage_SubscribePublishedFile( PublishedFileId_t unPublishedFileId )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_SubscribePublishedFile( self, unPublishedFileId );
}

SteamAPICall_t Steam_RemoteStorage_UGCDownload( UGCHandle_t hContent, uint32 unPriority )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UGCDownload( self, hContent, unPriority );
}

SteamAPICall_t Steam_RemoteStorage_UGCDownloadToLocation( UGCHandle_t hContent, const char * pchLocation, uint32 unPriority )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UGCDownloadToLocation( self, hContent, pchLocation, unPriority );
}

SteamAPICall_t Steam_RemoteStorage_UnsubscribePublishedFile( PublishedFileId_t unPublishedFileId )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UnsubscribePublishedFile( self, unPublishedFileId );
}

bool Steam_RemoteStorage_UpdatePublishedFileDescription( PublishedFileUpdateHandle_t updateHandle, const char * pchDescription )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UpdatePublishedFileDescription( self, updateHandle, pchDescription );
}

bool Steam_RemoteStorage_UpdatePublishedFileFile( PublishedFileUpdateHandle_t updateHandle, const char * pchFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UpdatePublishedFileFile( self, updateHandle, pchFile );
}

bool Steam_RemoteStorage_UpdatePublishedFilePreviewFile( PublishedFileUpdateHandle_t updateHandle, const char * pchPreviewFile )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UpdatePublishedFilePreviewFile( self, updateHandle, pchPreviewFile );
}

bool Steam_RemoteStorage_UpdatePublishedFileSetChangeDescription( PublishedFileUpdateHandle_t updateHandle, const char * pchChangeDescription )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription( self, updateHandle, pchChangeDescription );
}

bool Steam_RemoteStorage_UpdatePublishedFileTitle( PublishedFileUpdateHandle_t updateHandle, const char * pchTitle )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTitle( self, updateHandle, pchTitle );
}

bool Steam_RemoteStorage_UpdatePublishedFileVisibility( PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UpdatePublishedFileVisibility( self, updateHandle, eVisibility );
}

SteamAPICall_t Steam_RemoteStorage_UpdateUserPublishedItemVote( PublishedFileId_t unPublishedFileId, bool bVoteUp )
{
	auto *self = SteamAPI_SteamRemoteStorage_v016();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamRemoteStorage_UpdateUserPublishedItemVote( self, unPublishedFileId, bVoteUp );
}

ScreenshotHandle Steam_Screenshots_AddScreenshotToLibrary( const char * pchFilename, const char * pchThumbnailFilename, int nWidth, int nHeight )
{
	auto *self = SteamAPI_SteamScreenshots_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamScreenshots_AddScreenshotToLibrary( self, pchFilename, pchThumbnailFilename, nWidth, nHeight );
}

ScreenshotHandle Steam_Screenshots_AddVRScreenshotToLibrary( EVRScreenshotType eType, const char * pchFilename, const char * pchVRFilename )
{
	auto *self = SteamAPI_SteamScreenshots_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamScreenshots_AddVRScreenshotToLibrary( self, eType, pchFilename, pchVRFilename );
}

void Steam_Screenshots_HookScreenshots( bool bHook )
{
	auto *self = SteamAPI_SteamScreenshots_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamScreenshots_HookScreenshots( self, bHook );
}

bool Steam_Screenshots_IsScreenshotsHooked(  )
{
	auto *self = SteamAPI_SteamScreenshots_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamScreenshots_IsScreenshotsHooked( self );
}

bool Steam_Screenshots_SetLocation( ScreenshotHandle hScreenshot, const char * pchLocation )
{
	auto *self = SteamAPI_SteamScreenshots_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamScreenshots_SetLocation( self, hScreenshot, pchLocation );
}

bool Steam_Screenshots_TagPublishedFile( ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID )
{
	auto *self = SteamAPI_SteamScreenshots_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamScreenshots_TagPublishedFile( self, hScreenshot, unPublishedFileID );
}

bool Steam_Screenshots_TagUser( ScreenshotHandle hScreenshot, uint64_steamid steamID )
{
	auto *self = SteamAPI_SteamScreenshots_v003();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamScreenshots_TagUser( self, hScreenshot, steamID );
}

void Steam_Screenshots_TriggerScreenshot(  )
{
	auto *self = SteamAPI_SteamScreenshots_v003();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamScreenshots_TriggerScreenshot( self );
}

void Steam_Timeline_AddGamePhaseTag( const char * pchTagName, const char * pchTagIcon, const char * pchTagGroup, uint32 unPriority )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_AddGamePhaseTag( self, pchTagName, pchTagIcon, pchTagGroup, unPriority );
}

TimelineEventHandle_t Steam_Timeline_AddInstantaneousTimelineEvent( const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamTimeline_AddInstantaneousTimelineEvent( self, pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, ePossibleClip );
}

TimelineEventHandle_t Steam_Timeline_AddRangeTimelineEvent( const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, float flDuration, ETimelineEventClipPriority ePossibleClip )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamTimeline_AddRangeTimelineEvent( self, pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, flDuration, ePossibleClip );
}

void Steam_Timeline_ClearTimelineTooltip( float flTimeDelta )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_ClearTimelineTooltip( self, flTimeDelta );
}

SteamAPICall_t Steam_Timeline_DoesEventRecordingExist( TimelineEventHandle_t ulEvent )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamTimeline_DoesEventRecordingExist( self, ulEvent );
}

SteamAPICall_t Steam_Timeline_DoesGamePhaseRecordingExist( const char * pchPhaseID )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamTimeline_DoesGamePhaseRecordingExist( self, pchPhaseID );
}

void Steam_Timeline_EndGamePhase(  )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_EndGamePhase( self );
}

void Steam_Timeline_EndRangeTimelineEvent( TimelineEventHandle_t ulEvent, float flEndOffsetSeconds )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_EndRangeTimelineEvent( self, ulEvent, flEndOffsetSeconds );
}

void Steam_Timeline_OpenOverlayToGamePhase( const char * pchPhaseID )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_OpenOverlayToGamePhase( self, pchPhaseID );
}

void Steam_Timeline_OpenOverlayToTimelineEvent( const TimelineEventHandle_t ulEvent )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_OpenOverlayToTimelineEvent( self, ulEvent );
}

void Steam_Timeline_RemoveTimelineEvent( TimelineEventHandle_t ulEvent )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_RemoveTimelineEvent( self, ulEvent );
}

void Steam_Timeline_SetGamePhaseAttribute( const char * pchAttributeGroup, const char * pchAttributeValue, uint32 unPriority )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_SetGamePhaseAttribute( self, pchAttributeGroup, pchAttributeValue, unPriority );
}

void Steam_Timeline_SetGamePhaseID( const char * pchPhaseID )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_SetGamePhaseID( self, pchPhaseID );
}

void Steam_Timeline_SetTimelineGameMode( ETimelineGameMode eMode )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_SetTimelineGameMode( self, eMode );
}

void Steam_Timeline_SetTimelineTooltip( const char * pchDescription, float flTimeDelta )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_SetTimelineTooltip( self, pchDescription, flTimeDelta );
}

void Steam_Timeline_StartGamePhase(  )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_StartGamePhase( self );
}

TimelineEventHandle_t Steam_Timeline_StartRangeTimelineEvent( const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32 unPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamTimeline_StartRangeTimelineEvent( self, pchTitle, pchDescription, pchIcon, unPriority, flStartOffsetSeconds, ePossibleClip );
}

void Steam_Timeline_UpdateRangeTimelineEvent( TimelineEventHandle_t ulEvent, const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32 unPriority, ETimelineEventClipPriority ePossibleClip )
{
	auto *self = SteamAPI_SteamTimeline_v004();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamTimeline_UpdateRangeTimelineEvent( self, ulEvent, pchTitle, pchDescription, pchIcon, unPriority, ePossibleClip );
}

SteamAPICall_t Steam_UGC_AddAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddAppDependency( self, nPublishedFileID, nAppID );
}

bool Steam_UGC_AddContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddContentDescriptor( self, handle, descid );
}

SteamAPICall_t Steam_UGC_AddDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddDependency( self, nParentPublishedFileID, nChildPublishedFileID );
}

bool Steam_UGC_AddExcludedTag( UGCQueryHandle_t handle, const char * pTagName )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddExcludedTag( self, handle, pTagName );
}

bool Steam_UGC_AddItemKeyValueTag( UGCUpdateHandle_t handle, const char * pchKey, const char * pchValue )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddItemKeyValueTag( self, handle, pchKey, pchValue );
}

bool Steam_UGC_AddItemPreviewFile( UGCUpdateHandle_t handle, const char * pszPreviewFile, EItemPreviewType type )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddItemPreviewFile( self, handle, pszPreviewFile, type );
}

bool Steam_UGC_AddItemPreviewVideo( UGCUpdateHandle_t handle, const char * pszVideoID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddItemPreviewVideo( self, handle, pszVideoID );
}

SteamAPICall_t Steam_UGC_AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddItemToFavorites( self, nAppId, nPublishedFileID );
}

bool Steam_UGC_AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char * pKey, const char * pValue )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddRequiredKeyValueTag( self, handle, pKey, pValue );
}

bool Steam_UGC_AddRequiredTag( UGCQueryHandle_t handle, const char * pTagName )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_AddRequiredTag( self, handle, pTagName );
}

bool Steam_UGC_BInitWorkshopForGameServer( DepotId_t unWorkshopDepotID, const char * pszFolder )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_BInitWorkshopForGameServer( self, unWorkshopDepotID, pszFolder );
}

SteamAPICall_t Steam_UGC_CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_CreateItem( self, nConsumerAppId, eFileType );
}

UGCQueryHandle_t Steam_UGC_CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_CreateQueryAllUGCRequestPage( self, eQueryType, eMatchingeMatchingUGCTypeFileType, nCreatorAppID, nConsumerAppID, unPage );
}

UGCQueryHandle_t Steam_UGC_CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, const char * pchCursor )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_CreateQueryAllUGCRequestCursor( self, eQueryType, eMatchingeMatchingUGCTypeFileType, nCreatorAppID, nConsumerAppID, pchCursor );
}

UGCQueryHandle_t Steam_UGC_CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_CreateQueryUserUGCRequest( self, unAccountID, eListType, eMatchingUGCType, eSortOrder, nCreatorAppID, nConsumerAppID, unPage );
}

SteamAPICall_t Steam_UGC_DeleteItem( PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_DeleteItem( self, nPublishedFileID );
}

bool Steam_UGC_DownloadItem( PublishedFileId_t nPublishedFileID, bool bHighPriority )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_DownloadItem( self, nPublishedFileID, bHighPriority );
}

SteamAPICall_t Steam_UGC_GetAppDependencies( PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetAppDependencies( self, nPublishedFileID );
}

uint32 Steam_UGC_GetItemState( PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetItemState( self, nPublishedFileID );
}

uint32 Steam_UGC_GetNumDownloadedItems(  )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetNumDownloadedItems( self );
}

uint32 Steam_UGC_GetNumSubscribedItems( bool bIncludeLocallyDisabled )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetNumSubscribedItems( self, bIncludeLocallyDisabled );
}

uint32 Steam_UGC_GetNumSupportedGameVersions( UGCQueryHandle_t handle, uint32 index )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetNumSupportedGameVersions( self, handle, index );
}

uint32 Steam_UGC_GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32 index )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetQueryUGCNumAdditionalPreviews( self, handle, index );
}

uint32 Steam_UGC_GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32 index )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetQueryUGCNumKeyValueTags( self, handle, index );
}

uint32 Steam_UGC_GetQueryUGCNumTags( UGCQueryHandle_t handle, uint32 index )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetQueryUGCNumTags( self, handle, index );
}

SteamAPICall_t Steam_UGC_GetUserItemVote( PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetUserItemVote( self, nPublishedFileID );
}

SteamAPICall_t Steam_UGC_GetWorkshopEULAStatus(  )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_GetWorkshopEULAStatus( self );
}

bool Steam_UGC_MarkDownloadedItemAsUnused( PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_MarkDownloadedItemAsUnused( self, nPublishedFileID );
}

bool Steam_UGC_ReleaseQueryUGCRequest( UGCQueryHandle_t handle )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_ReleaseQueryUGCRequest( self, handle );
}

bool Steam_UGC_RemoveAllItemKeyValueTags( UGCUpdateHandle_t handle )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_RemoveAllItemKeyValueTags( self, handle );
}

SteamAPICall_t Steam_UGC_RemoveAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_RemoveAppDependency( self, nPublishedFileID, nAppID );
}

bool Steam_UGC_RemoveContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_RemoveContentDescriptor( self, handle, descid );
}

SteamAPICall_t Steam_UGC_RemoveDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_RemoveDependency( self, nParentPublishedFileID, nChildPublishedFileID );
}

SteamAPICall_t Steam_UGC_RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_RemoveItemFromFavorites( self, nAppId, nPublishedFileID );
}

bool Steam_UGC_RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char * pchKey )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_RemoveItemKeyValueTags( self, handle, pchKey );
}

bool Steam_UGC_RemoveItemPreview( UGCUpdateHandle_t handle, uint32 index )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_RemoveItemPreview( self, handle, index );
}

SteamAPICall_t Steam_UGC_RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_RequestUGCDetails( self, nPublishedFileID, unMaxAgeSeconds );
}

SteamAPICall_t Steam_UGC_SendQueryUGCRequest( UGCQueryHandle_t handle )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SendQueryUGCRequest( self, handle );
}

bool Steam_UGC_SetAdminQuery( UGCUpdateHandle_t handle, bool bAdminQuery )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetAdminQuery( self, handle, bAdminQuery );
}

bool Steam_UGC_SetAllowCachedResponse( UGCQueryHandle_t handle, uint32 unMaxAgeSeconds )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetAllowCachedResponse( self, handle, unMaxAgeSeconds );
}

bool Steam_UGC_SetAllowLegacyUpload( UGCUpdateHandle_t handle, bool bAllowLegacyUpload )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetAllowLegacyUpload( self, handle, bAllowLegacyUpload );
}

bool Steam_UGC_SetCloudFileNameFilter( UGCQueryHandle_t handle, const char * pMatchCloudFileName )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetCloudFileNameFilter( self, handle, pMatchCloudFileName );
}

bool Steam_UGC_SetItemContent( UGCUpdateHandle_t handle, const char * pszContentFolder )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetItemContent( self, handle, pszContentFolder );
}

bool Steam_UGC_SetItemDescription( UGCUpdateHandle_t handle, const char * pchDescription )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetItemDescription( self, handle, pchDescription );
}

bool Steam_UGC_SetItemMetadata( UGCUpdateHandle_t handle, const char * pchMetaData )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetItemMetadata( self, handle, pchMetaData );
}

bool Steam_UGC_SetItemPreview( UGCUpdateHandle_t handle, const char * pszPreviewFile )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetItemPreview( self, handle, pszPreviewFile );
}

bool Steam_UGC_SetItemTitle( UGCUpdateHandle_t handle, const char * pchTitle )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetItemTitle( self, handle, pchTitle );
}

bool Steam_UGC_SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char * pchLanguage )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetItemUpdateLanguage( self, handle, pchLanguage );
}

bool Steam_UGC_SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetItemVisibility( self, handle, eVisibility );
}

bool Steam_UGC_SetLanguage( UGCQueryHandle_t handle, const char * pchLanguage )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetLanguage( self, handle, pchLanguage );
}

bool Steam_UGC_SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetMatchAnyTag( self, handle, bMatchAnyTag );
}

bool Steam_UGC_SetRankedByTrendDays( UGCQueryHandle_t handle, uint32 unDays )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetRankedByTrendDays( self, handle, unDays );
}

bool Steam_UGC_SetRequiredGameVersions( UGCUpdateHandle_t handle, const char * pszGameBranchMin, const char * pszGameBranchMax )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetRequiredGameVersions( self, handle, pszGameBranchMin, pszGameBranchMax );
}

bool Steam_UGC_SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetReturnAdditionalPreviews( self, handle, bReturnAdditionalPreviews );
}

bool Steam_UGC_SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetReturnChildren( self, handle, bReturnChildren );
}

bool Steam_UGC_SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetReturnKeyValueTags( self, handle, bReturnKeyValueTags );
}

bool Steam_UGC_SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetReturnLongDescription( self, handle, bReturnLongDescription );
}

bool Steam_UGC_SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetReturnMetadata( self, handle, bReturnMetadata );
}

bool Steam_UGC_SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetReturnOnlyIDs( self, handle, bReturnOnlyIDs );
}

bool Steam_UGC_SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32 unDays )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetReturnPlaytimeStats( self, handle, unDays );
}

bool Steam_UGC_SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetReturnTotalOnly( self, handle, bReturnTotalOnly );
}

bool Steam_UGC_SetSearchText( UGCQueryHandle_t handle, const char * pSearchText )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetSearchText( self, handle, pSearchText );
}

bool Steam_UGC_SetTimeCreatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetTimeCreatedDateRange( self, handle, rtStart, rtEnd );
}

bool Steam_UGC_SetTimeUpdatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetTimeUpdatedDateRange( self, handle, rtStart, rtEnd );
}

SteamAPICall_t Steam_UGC_SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SetUserItemVote( self, nPublishedFileID, bVoteUp );
}

bool Steam_UGC_ShowWorkshopEULA(  )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_ShowWorkshopEULA( self );
}

UGCUpdateHandle_t Steam_UGC_StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_StartItemUpdate( self, nConsumerAppId, nPublishedFileID );
}

SteamAPICall_t Steam_UGC_StopPlaytimeTrackingForAllItems(  )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_StopPlaytimeTrackingForAllItems( self );
}

SteamAPICall_t Steam_UGC_SubmitItemUpdate( UGCUpdateHandle_t handle, const char * pchChangeNote )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SubmitItemUpdate( self, handle, pchChangeNote );
}

SteamAPICall_t Steam_UGC_SubscribeItem( PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_SubscribeItem( self, nPublishedFileID );
}

void Steam_UGC_SuspendDownloads( bool bSuspend )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUGC_SuspendDownloads( self, bSuspend );
}

SteamAPICall_t Steam_UGC_UnsubscribeItem( PublishedFileId_t nPublishedFileID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_UnsubscribeItem( self, nPublishedFileID );
}

bool Steam_UGC_UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32 index, const char * pszPreviewFile )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_UpdateItemPreviewFile( self, handle, index, pszPreviewFile );
}

bool Steam_UGC_UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32 index, const char * pszVideoID )
{
	auto *self = SteamAPI_SteamUGC_v021();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUGC_UpdateItemPreviewVideo( self, handle, index, pszVideoID );
}

SteamAPICall_t Steam_UserStats_AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_AttachLeaderboardUGC( self, hSteamLeaderboard, hUGC );
}

bool Steam_UserStats_ClearAchievement( const char * pchName )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_ClearAchievement( self, pchName );
}

SteamAPICall_t Steam_UserStats_DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_DownloadLeaderboardEntries( self, hSteamLeaderboard, eLeaderboardDataRequest, nRangeStart, nRangeEnd );
}

SteamAPICall_t Steam_UserStats_FindLeaderboard( const char * pchLeaderboardName )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_FindLeaderboard( self, pchLeaderboardName );
}

SteamAPICall_t Steam_UserStats_FindOrCreateLeaderboard( const char * pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_FindOrCreateLeaderboard( self, pchLeaderboardName, eLeaderboardSortMethod, eLeaderboardDisplayType );
}

const char * Steam_UserStats_GetAchievementDisplayAttribute( const char * pchName, const char * pchKey )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute( self, pchName, pchKey );
}

int Steam_UserStats_GetAchievementIcon( const char * pchName )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetAchievementIcon( self, pchName );
}

const char * Steam_UserStats_GetAchievementName( uint32 iAchievement )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetAchievementName( self, iAchievement );
}

ELeaderboardDisplayType Steam_UserStats_GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetLeaderboardDisplayType( self, hSteamLeaderboard );
}

int Steam_UserStats_GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetLeaderboardEntryCount( self, hSteamLeaderboard );
}

const char * Steam_UserStats_GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetLeaderboardName( self, hSteamLeaderboard );
}

ELeaderboardSortMethod Steam_UserStats_GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetLeaderboardSortMethod( self, hSteamLeaderboard );
}

uint32 Steam_UserStats_GetNumAchievements(  )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetNumAchievements( self );
}

SteamAPICall_t Steam_UserStats_GetNumberOfCurrentPlayers(  )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_GetNumberOfCurrentPlayers( self );
}

bool Steam_UserStats_IndicateAchievementProgress( const char * pchName, uint32 nCurProgress, uint32 nMaxProgress )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_IndicateAchievementProgress( self, pchName, nCurProgress, nMaxProgress );
}

SteamAPICall_t Steam_UserStats_RequestGlobalAchievementPercentages(  )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_RequestGlobalAchievementPercentages( self );
}

SteamAPICall_t Steam_UserStats_RequestGlobalStats( int nHistoryDays )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_RequestGlobalStats( self, nHistoryDays );
}

SteamAPICall_t Steam_UserStats_RequestUserStats( uint64_steamid steamIDUser )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_RequestUserStats( self, steamIDUser );
}

bool Steam_UserStats_ResetAllStats( bool bAchievementsToo )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_ResetAllStats( self, bAchievementsToo );
}

bool Steam_UserStats_SetAchievement( const char * pchName )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_SetAchievement( self, pchName );
}

bool Steam_UserStats_SetStat( const char * pchName, int32 nData )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_SetStatInt32( self, pchName, nData );
}

bool Steam_UserStats_SetStat( const char * pchName, float fData )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_SetStatFloat( self, pchName, fData );
}

bool Steam_UserStats_StoreStats(  )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_StoreStats( self );
}

bool Steam_UserStats_UpdateAvgRateStat( const char * pchName, float flCountThisSession, double dSessionLength )
{
	auto *self = SteamAPI_SteamUserStats_v013();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUserStats_UpdateAvgRateStat( self, pchName, flCountThisSession, dSessionLength );
}

void Steam_User_AdvertiseGame( uint64_steamid steamIDGameServer, uint32 unIPServer, uint16 usPortServer )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUser_AdvertiseGame( self, steamIDGameServer, unIPServer, usPortServer );
}

bool Steam_User_BIsBehindNAT(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_BIsBehindNAT( self );
}

bool Steam_User_BIsPhoneIdentifying(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_BIsPhoneIdentifying( self );
}

bool Steam_User_BIsPhoneRequiringVerification(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_BIsPhoneRequiringVerification( self );
}

bool Steam_User_BIsPhoneVerified(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_BIsPhoneVerified( self );
}

bool Steam_User_BIsTwoFactorEnabled(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_BIsTwoFactorEnabled( self );
}

bool Steam_User_BLoggedOn(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_BLoggedOn( self );
}

bool Steam_User_BSetDurationControlOnlineState( EDurationControlOnlineState eNewState )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_BSetDurationControlOnlineState( self, eNewState );
}

void Steam_User_CancelAuthTicket( HAuthTicket hAuthTicket )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUser_CancelAuthTicket( self, hAuthTicket );
}

void Steam_User_EndAuthSession( uint64_steamid steamID )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUser_EndAuthSession( self, steamID );
}

HAuthTicket Steam_User_GetAuthTicketForWebApi( const char * pchIdentity )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_GetAuthTicketForWebApi( self, pchIdentity );
}

SteamAPICall_t Steam_User_GetDurationControl(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_GetDurationControl( self );
}

int Steam_User_GetGameBadgeLevel( int nSeries, bool bFoil )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_GetGameBadgeLevel( self, nSeries, bFoil );
}

HSteamUser Steam_User_GetHSteamUser(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_GetHSteamUser( self );
}

SteamAPICall_t Steam_User_GetMarketEligibility(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_GetMarketEligibility( self );
}

int Steam_User_GetPlayerSteamLevel(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_GetPlayerSteamLevel( self );
}

uint64_steamid Steam_User_GetSteamID(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_GetSteamID( self );
}

uint32 Steam_User_GetVoiceOptimalSampleRate(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_GetVoiceOptimalSampleRate( self );
}

SteamAPICall_t Steam_User_RequestStoreAuthURL( const char * pchRedirectURL )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_RequestStoreAuthURL( self, pchRedirectURL );
}

void Steam_User_StartVoiceRecording(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUser_StartVoiceRecording( self );
}

void Steam_User_StopVoiceRecording(  )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUser_StopVoiceRecording( self );
}

void Steam_User_TerminateGameConnection_DEPRECATED( uint32 unIPServer, uint16 usPortServer )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUser_TerminateGameConnection_DEPRECATED( self, unIPServer, usPortServer );
}

void Steam_User_TrackAppUsageEvent( uint64_gameid gameID, int eAppUsageEvent, const char * pchExtraInfo )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUser_TrackAppUsageEvent( self, gameID, eAppUsageEvent, pchExtraInfo );
}

EUserHasLicenseForAppResult Steam_User_UserHasLicenseForApp( uint64_steamid steamID, AppId_t appID )
{
	auto *self = SteamAPI_SteamUser_v023();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUser_UserHasLicenseForApp( self, steamID, appID );
}

bool Steam_Utils_BOverlayNeedsPresent(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_BOverlayNeedsPresent( self );
}

SteamAPICall_t Steam_Utils_CheckFileSignature( const char * szFileName )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_CheckFileSignature( self, szFileName );
}

bool Steam_Utils_DismissFloatingGamepadTextInput(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_DismissFloatingGamepadTextInput( self );
}

bool Steam_Utils_DismissGamepadTextInput(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_DismissGamepadTextInput( self );
}

ESteamAPICallFailure Steam_Utils_GetAPICallFailureReason( SteamAPICall_t hSteamAPICall )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetAPICallFailureReason( self, hSteamAPICall );
}

uint32 Steam_Utils_GetAppID(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetAppID( self );
}

EUniverse Steam_Utils_GetConnectedUniverse(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetConnectedUniverse( self );
}

uint8 Steam_Utils_GetCurrentBatteryPower(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetCurrentBatteryPower( self );
}

uint32 Steam_Utils_GetEnteredGamepadTextLength(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetEnteredGamepadTextLength( self );
}

uint32 Steam_Utils_GetIPCCallCount(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetIPCCallCount( self );
}

const char * Steam_Utils_GetIPCountry(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetIPCountry( self );
}

ESteamIPv6ConnectivityState Steam_Utils_GetIPv6ConnectivityState( ESteamIPv6ConnectivityProtocol eProtocol )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetIPv6ConnectivityState( self, eProtocol );
}

uint32 Steam_Utils_GetSecondsSinceAppActive(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetSecondsSinceAppActive( self );
}

uint32 Steam_Utils_GetSecondsSinceComputerActive(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetSecondsSinceComputerActive( self );
}

uint32 Steam_Utils_GetServerRealTime(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetServerRealTime( self );
}

ESteamHardwareDefaultConfig Steam_Utils_GetSteamHardwareDefaultConfig(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetSteamHardwareDefaultConfig( self );
}

const char * Steam_Utils_GetSteamUILanguage(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_GetSteamUILanguage( self );
}

bool Steam_Utils_InitFilterText( uint32 unFilterOptions )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_InitFilterText( self, unFilterOptions );
}

bool Steam_Utils_IsOverlayEnabled(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_IsOverlayEnabled( self );
}

ESteamHardwareType Steam_Utils_IsRunningOnSteamHardware(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_IsRunningOnSteamHardware( self );
}

bool Steam_Utils_IsRunningUnderProton(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_IsRunningUnderProton( self );
}

bool Steam_Utils_IsSteamChinaLauncher(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_IsSteamChinaLauncher( self );
}

bool Steam_Utils_IsSteamInBigPictureMode(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_IsSteamInBigPictureMode( self );
}

bool Steam_Utils_IsSteamRunningInVR(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_IsSteamRunningInVR( self );
}

bool Steam_Utils_IsVRHeadsetStreamingEnabled(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_IsVRHeadsetStreamingEnabled( self );
}

void Steam_Utils_SetGameLauncherMode( bool bLauncherMode )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUtils_SetGameLauncherMode( self, bLauncherMode );
}

void Steam_Utils_SetOverlayNotificationInset( int nHorizontalInset, int nVerticalInset )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUtils_SetOverlayNotificationInset( self, nHorizontalInset, nVerticalInset );
}

void Steam_Utils_SetOverlayNotificationPosition( ENotificationPosition eNotificationPosition )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUtils_SetOverlayNotificationPosition( self, eNotificationPosition );
}

void Steam_Utils_SetVRHeadsetStreamingEnabled( bool bEnabled )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUtils_SetVRHeadsetStreamingEnabled( self, bEnabled );
}

void Steam_Utils_SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUtils_SetWarningMessageHook( self, pFunction );
}

bool Steam_Utils_ShowFloatingGamepadTextInput( EFloatingGamepadTextInputMode eKeyboardMode, int nTextFieldXPosition, int nTextFieldYPosition, int nTextFieldWidth, int nTextFieldHeight )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput( self, eKeyboardMode, nTextFieldXPosition, nTextFieldYPosition, nTextFieldWidth, nTextFieldHeight );
}

bool Steam_Utils_ShowGamepadTextInput( EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char * pchDescription, uint32 unCharMax, const char * pchExistingText )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return {};
	}
	return SteamAPI_ISteamUtils_ShowGamepadTextInput( self, eInputMode, eLineInputMode, pchDescription, unCharMax, pchExistingText );
}

void Steam_Utils_StartVRDashboard(  )
{
	auto *self = SteamAPI_SteamUtils_v011();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamUtils_StartVRDashboard( self );
}

void Steam_Video_GetOPFSettings( AppId_t unVideoAppID )
{
	auto *self = SteamAPI_SteamVideo_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamVideo_GetOPFSettings( self, unVideoAppID );
}

void Steam_Video_GetVideoURL( AppId_t unVideoAppID )
{
	auto *self = SteamAPI_SteamVideo_v007();
	if ( !self )
	{
		return;
	}
	SteamAPI_ISteamVideo_GetVideoURL( self, unVideoAppID );
}

