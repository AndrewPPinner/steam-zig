#define SWS_BUILD_SHARED 1
#include "steamworks_c_api.h"
#include "steamworks_helpers.h"

#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <vector>

namespace
{
SWS_String CopyStringForC( const std::string &value )
{
	SWS_String result = {};
	char *data = static_cast<char *>( std::malloc( value.size() + 1 ) );
	if ( !data )
	{
		return result;
	}
	std::memcpy( data, value.data(), value.size() );
	data[value.size()] = '\0';
	result.data = data;
	result.size = value.size();
	return result;
}

SWS_StringList CopyStringListForC( const std::vector<std::string> &values )
{
	SWS_StringList result = {};
	if ( values.empty() )
	{
		return result;
	}

	SWS_String *items = static_cast<SWS_String *>( std::calloc( values.size(), sizeof( SWS_String ) ) );
	if ( !items )
	{
		return result;
	}
	for ( size_t index = 0; index < values.size(); ++index )
	{
		items[index] = CopyStringForC( values[index] );
	}
	result.items = items;
	result.size = values.size();
	return result;
}

SWS_Bytes CopyBytesForC( const std::string &value )
{
	SWS_Bytes result = {};
	uint8_t *data = static_cast<uint8_t *>( std::malloc( value.size() ) );
	if ( !data && !value.empty() )
	{
		return result;
	}
	if ( !value.empty() )
	{
		std::memcpy( data, value.data(), value.size() );
	}
	result.data = data;
	result.size = value.size();
	return result;
}

SWS_BytesList CopyBytesListForC( const std::vector<std::string> &values )
{
	SWS_BytesList result = {};
	if ( values.empty() )
	{
		return result;
	}

	SWS_Bytes *items = static_cast<SWS_Bytes *>( std::calloc( values.size(), sizeof( SWS_Bytes ) ) );
	if ( !items )
	{
		return result;
	}
	for ( size_t index = 0; index < values.size(); ++index )
	{
		items[index] = CopyBytesForC( values[index] );
	}
	result.items = items;
	result.size = values.size();
	return result;
}

const char *EmptyCString()
{
	return "";
}
}

void SWS_FreeString( SWS_String value )
{
	std::free( const_cast<char *>( value.data ) );
}

void SWS_FreeStringList( SWS_StringList value )
{
	for ( size_t index = 0; index < value.size; ++index )
	{
		SWS_FreeString( value.items[index] );
	}
	std::free( value.items );
}

void SWS_FreeBytes( SWS_Bytes value )
{
	std::free( const_cast<uint8_t *>( value.data ) );
}

void SWS_FreeBytesList( SWS_BytesList value )
{
	for ( size_t index = 0; index < value.size; ++index )
	{
		SWS_FreeBytes( value.items[index] );
	}
	std::free( value.items );
}

size_t SWS_StringList_Size( SWS_StringList value )
{
	return value.size;
}

SWS_String SWS_StringList_Get( SWS_StringList value, size_t index )
{
	if ( index >= value.size || !value.items )
	{
		return {};
	}
	return value.items[index];
}

size_t SWS_BytesList_Size( SWS_BytesList value )
{
	return value.size;
}

SWS_Bytes SWS_BytesList_Get( SWS_BytesList value, size_t index )
{
	if ( index >= value.size || !value.items )
	{
		return {};
	}
	return value.items[index];
}

bool SWS_SteamAPI_ISteamApps_BIsAppInstalled( uint32_t appID )
{
	try
	{
		return Steam_Apps_BIsAppInstalled( static_cast<AppId_t>( appID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_BIsCybercafe( void )
{
	try
	{
		return Steam_Apps_BIsCybercafe();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_BIsDlcInstalled( uint32_t appID )
{
	try
	{
		return Steam_Apps_BIsDlcInstalled( static_cast<AppId_t>( appID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_BIsLowViolence( void )
{
	try
	{
		return Steam_Apps_BIsLowViolence();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_BIsSubscribed( void )
{
	try
	{
		return Steam_Apps_BIsSubscribed();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_BIsSubscribedApp( uint32_t appID )
{
	try
	{
		return Steam_Apps_BIsSubscribedApp( static_cast<AppId_t>( appID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_BIsSubscribedFromFamilySharing( void )
{
	try
	{
		return Steam_Apps_BIsSubscribedFromFamilySharing();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_BIsSubscribedFromFreeWeekend( void )
{
	try
	{
		return Steam_Apps_BIsSubscribedFromFreeWeekend();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_BIsVACBanned( void )
{
	try
	{
		return Steam_Apps_BIsVACBanned();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamApps_GetAppBuildId( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Apps_GetAppBuildId() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamApps_GetAppOwner( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_Apps_GetAppOwner() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamApps_GetAvailableGameLanguages( void )
{
	try
	{
		return Steam_Apps_GetAvailableGameLanguages();
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamApps_GetCurrentGameLanguage( void )
{
	try
	{
		return Steam_Apps_GetCurrentGameLanguage();
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamApps_GetDLCCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Apps_GetDLCCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime( uint32_t nAppID )
{
	try
	{
		return static_cast<uint32_t>( Steam_Apps_GetEarliestPurchaseUnixTime( static_cast<AppId_t>( nAppID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamApps_GetFileDetails( const char * pszFileName )
{
	try
	{
		return static_cast<uint64_t>( Steam_Apps_GetFileDetails( pszFileName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamApps_GetLaunchQueryParam( const char * pchKey )
{
	try
	{
		return Steam_Apps_GetLaunchQueryParam( pchKey );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

void SWS_SteamAPI_ISteamApps_InstallDLC( uint32_t nAppID )
{
	try
	{
		Steam_Apps_InstallDLC( static_cast<AppId_t>( nAppID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamApps_MarkContentCorrupt( bool bMissingFilesOnly )
{
	try
	{
		return Steam_Apps_MarkContentCorrupt( bMissingFilesOnly );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamApps_RequestAllProofOfPurchaseKeys( void )
{
	try
	{
		Steam_Apps_RequestAllProofOfPurchaseKeys();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamApps_RequestAppProofOfPurchaseKey( uint32_t nAppID )
{
	try
	{
		Steam_Apps_RequestAppProofOfPurchaseKey( static_cast<AppId_t>( nAppID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamApps_SetActiveBeta( const char * pchBetaName )
{
	try
	{
		return Steam_Apps_SetActiveBeta( pchBetaName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamApps_SetDlcContext( uint32_t nAppID )
{
	try
	{
		return Steam_Apps_SetDlcContext( static_cast<AppId_t>( nAppID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamApps_SetGamePerformanceSetting( int32_t setting )
{
	try
	{
		Steam_Apps_SetGamePerformanceSetting( static_cast<EGamePerformanceSetting>( setting ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamApps_SetGameRenderResolution( uint32_t unWidth, uint32_t unHeight )
{
	try
	{
		Steam_Apps_SetGameRenderResolution( static_cast<uint32>( unWidth ), static_cast<uint32>( unHeight ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamApps_UninstallDLC( uint32_t nAppID )
{
	try
	{
		Steam_Apps_UninstallDLC( static_cast<AppId_t>( nAppID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamController_ActivateActionSet( uint64_t controllerHandle, uint64_t actionSetHandle )
{
	try
	{
		Steam_Controller_ActivateActionSet( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<ControllerActionSetHandle_t>( actionSetHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamController_ActivateActionSetLayer( uint64_t controllerHandle, uint64_t actionSetLayerHandle )
{
	try
	{
		Steam_Controller_ActivateActionSetLayer( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<ControllerActionSetHandle_t>( actionSetLayerHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamController_DeactivateActionSetLayer( uint64_t controllerHandle, uint64_t actionSetLayerHandle )
{
	try
	{
		Steam_Controller_DeactivateActionSetLayer( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<ControllerActionSetHandle_t>( actionSetLayerHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamController_DeactivateAllActionSetLayers( uint64_t controllerHandle )
{
	try
	{
		Steam_Controller_DeactivateAllActionSetLayers( static_cast<ControllerHandle_t>( controllerHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamController_GetActionOriginFromXboxOrigin( uint64_t controllerHandle, int32_t eOrigin )
{
	try
	{
		return static_cast<int32_t>( Steam_Controller_GetActionOriginFromXboxOrigin( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<EXboxOrigin>( eOrigin ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamController_GetActionSetHandle( const char * pszActionSetName )
{
	try
	{
		return static_cast<uint64_t>( Steam_Controller_GetActionSetHandle( pszActionSetName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamController_GetAnalogActionHandle( const char * pszActionName )
{
	try
	{
		return static_cast<uint64_t>( Steam_Controller_GetAnalogActionHandle( pszActionName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamController_GetControllerForGamepadIndex( int32_t nIndex )
{
	try
	{
		return static_cast<uint64_t>( Steam_Controller_GetControllerForGamepadIndex( static_cast<int>( nIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamController_GetCurrentActionSet( uint64_t controllerHandle )
{
	try
	{
		return static_cast<uint64_t>( Steam_Controller_GetCurrentActionSet( static_cast<ControllerHandle_t>( controllerHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamController_GetDigitalActionHandle( const char * pszActionName )
{
	try
	{
		return static_cast<uint64_t>( Steam_Controller_GetDigitalActionHandle( pszActionName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamController_GetGamepadIndexForController( uint64_t ulControllerHandle )
{
	try
	{
		return static_cast<int32_t>( Steam_Controller_GetGamepadIndexForController( static_cast<ControllerHandle_t>( ulControllerHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamController_GetGlyphForActionOrigin( int32_t eOrigin )
{
	try
	{
		return Steam_Controller_GetGlyphForActionOrigin( static_cast<EControllerActionOrigin>( eOrigin ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamController_GetGlyphForXboxOrigin( int32_t eOrigin )
{
	try
	{
		return Steam_Controller_GetGlyphForXboxOrigin( static_cast<EXboxOrigin>( eOrigin ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamController_GetInputTypeForHandle( uint64_t controllerHandle )
{
	try
	{
		return static_cast<int32_t>( Steam_Controller_GetInputTypeForHandle( static_cast<ControllerHandle_t>( controllerHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamController_GetStringForActionOrigin( int32_t eOrigin )
{
	try
	{
		return Steam_Controller_GetStringForActionOrigin( static_cast<EControllerActionOrigin>( eOrigin ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamController_GetStringForXboxOrigin( int32_t eOrigin )
{
	try
	{
		return Steam_Controller_GetStringForXboxOrigin( static_cast<EXboxOrigin>( eOrigin ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

bool SWS_SteamAPI_ISteamController_Init( void )
{
	try
	{
		return Steam_Controller_Init();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamController_RunFrame( void )
{
	try
	{
		Steam_Controller_RunFrame();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamController_SetLEDColor( uint64_t controllerHandle, uint8_t nColorR, uint8_t nColorG, uint8_t nColorB, uint32_t nFlags )
{
	try
	{
		Steam_Controller_SetLEDColor( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<uint8>( nColorR ), static_cast<uint8>( nColorG ), static_cast<uint8>( nColorB ), static_cast<unsigned int>( nFlags ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamController_ShowBindingPanel( uint64_t controllerHandle )
{
	try
	{
		return Steam_Controller_ShowBindingPanel( static_cast<ControllerHandle_t>( controllerHandle ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamController_Shutdown( void )
{
	try
	{
		return Steam_Controller_Shutdown();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamController_StopAnalogActionMomentum( uint64_t controllerHandle, uint64_t eAction )
{
	try
	{
		Steam_Controller_StopAnalogActionMomentum( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<ControllerAnalogActionHandle_t>( eAction ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamController_TranslateActionOrigin( int32_t eDestinationInputType, int32_t eSourceOrigin )
{
	try
	{
		return static_cast<int32_t>( Steam_Controller_TranslateActionOrigin( static_cast<ESteamInputType>( eDestinationInputType ), static_cast<EControllerActionOrigin>( eSourceOrigin ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamController_TriggerHapticPulse( uint64_t controllerHandle, int32_t eTargetPad, uint16_t usDurationMicroSec )
{
	try
	{
		Steam_Controller_TriggerHapticPulse( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<ESteamControllerPad>( eTargetPad ), static_cast<unsigned short>( usDurationMicroSec ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamController_TriggerRepeatedHapticPulse( uint64_t controllerHandle, int32_t eTargetPad, uint16_t usDurationMicroSec, uint16_t usOffMicroSec, uint16_t unRepeat, uint32_t nFlags )
{
	try
	{
		Steam_Controller_TriggerRepeatedHapticPulse( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<ESteamControllerPad>( eTargetPad ), static_cast<unsigned short>( usDurationMicroSec ), static_cast<unsigned short>( usOffMicroSec ), static_cast<unsigned short>( unRepeat ), static_cast<unsigned int>( nFlags ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamController_TriggerVibration( uint64_t controllerHandle, uint16_t usLeftSpeed, uint16_t usRightSpeed )
{
	try
	{
		Steam_Controller_TriggerVibration( static_cast<ControllerHandle_t>( controllerHandle ), static_cast<unsigned short>( usLeftSpeed ), static_cast<unsigned short>( usRightSpeed ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamFriends_ActivateGameOverlay( const char * pchDialog )
{
	try
	{
		Steam_Friends_ActivateGameOverlay( pchDialog );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog( uint64_t steamIDLobby )
{
	try
	{
		Steam_Friends_ActivateGameOverlayInviteDialog( static_cast<uint64_steamid>( steamIDLobby ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialogConnectString( const char * pchConnectString )
{
	try
	{
		Steam_Friends_ActivateGameOverlayInviteDialogConnectString( pchConnectString );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamFriends_ActivateGameOverlayRemotePlayTogetherInviteDialog( uint64_t steamIDLobby )
{
	try
	{
		Steam_Friends_ActivateGameOverlayRemotePlayTogetherInviteDialog( static_cast<uint64_steamid>( steamIDLobby ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamFriends_ActivateGameOverlayToStore( uint32_t nAppID, int32_t eFlag )
{
	try
	{
		Steam_Friends_ActivateGameOverlayToStore( static_cast<AppId_t>( nAppID ), static_cast<EOverlayToStoreFlag>( eFlag ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamFriends_ActivateGameOverlayToUser( const char * pchDialog, uint64_t steamID )
{
	try
	{
		Steam_Friends_ActivateGameOverlayToUser( pchDialog, static_cast<uint64_steamid>( steamID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage( const char * pchURL, int32_t eMode )
{
	try
	{
		Steam_Friends_ActivateGameOverlayToWebPage( pchURL, static_cast<EActivateGameOverlayToWebPageMode>( eMode ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamFriends_BHasEquippedProfileItem( uint64_t steamID, int32_t itemType )
{
	try
	{
		return Steam_Friends_BHasEquippedProfileItem( static_cast<uint64_steamid>( steamID ), static_cast<ECommunityProfileItemType>( itemType ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamFriends_ClearRichPresence( void )
{
	try
	{
		Steam_Friends_ClearRichPresence();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamFriends_CloseClanChatWindowInSteam( uint64_t steamIDClanChat )
{
	try
	{
		return Steam_Friends_CloseClanChatWindowInSteam( static_cast<uint64_steamid>( steamIDClanChat ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_EnumerateFollowingList( uint32_t unStartIndex )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_EnumerateFollowingList( static_cast<uint32>( unStartIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_GetChatMemberByIndex( uint64_t steamIDClan, int32_t iUser )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetChatMemberByIndex( static_cast<uint64_steamid>( steamIDClan ), static_cast<int>( iUser ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_GetClanByIndex( int32_t iClan )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetClanByIndex( static_cast<int>( iClan ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetClanChatMemberCount( uint64_t steamIDClan )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetClanChatMemberCount( static_cast<uint64_steamid>( steamIDClan ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetClanCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetClanCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetClanName( uint64_t steamIDClan )
{
	try
	{
		return Steam_Friends_GetClanName( static_cast<uint64_steamid>( steamIDClan ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_GetClanOfficerByIndex( uint64_t steamIDClan, int32_t iOfficer )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetClanOfficerByIndex( static_cast<uint64_steamid>( steamIDClan ), static_cast<int>( iOfficer ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetClanOfficerCount( uint64_t steamIDClan )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetClanOfficerCount( static_cast<uint64_steamid>( steamIDClan ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_GetClanOwner( uint64_t steamIDClan )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetClanOwner( static_cast<uint64_steamid>( steamIDClan ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetClanTag( uint64_t steamIDClan )
{
	try
	{
		return Steam_Friends_GetClanTag( static_cast<uint64_steamid>( steamIDClan ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_GetCoplayFriend( int32_t iCoplayFriend )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetCoplayFriend( static_cast<int>( iCoplayFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetCoplayFriendCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetCoplayFriendCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_GetFollowerCount( uint64_t steamID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetFollowerCount( static_cast<uint64_steamid>( steamID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_GetFriendByIndex( int32_t iFriend, int32_t iFriendFlags )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetFriendByIndex( static_cast<int>( iFriend ), static_cast<int>( iFriendFlags ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamFriends_GetFriendCoplayGame( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<uint32_t>( Steam_Friends_GetFriendCoplayGame( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendCoplayTime( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendCoplayTime( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendCount( int32_t iFriendFlags )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendCount( static_cast<int>( iFriendFlags ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendCountFromSource( uint64_t steamIDSource )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendCountFromSource( static_cast<uint64_steamid>( steamIDSource ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_GetFriendFromSourceByIndex( uint64_t steamIDSource, int32_t iFriend )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetFriendFromSourceByIndex( static_cast<uint64_steamid>( steamIDSource ), static_cast<int>( iFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetFriendPersonaName( uint64_t steamIDFriend )
{
	try
	{
		return Steam_Friends_GetFriendPersonaName( static_cast<uint64_steamid>( steamIDFriend ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetFriendPersonaNameHistory( uint64_t steamIDFriend, int32_t iPersonaName )
{
	try
	{
		return Steam_Friends_GetFriendPersonaNameHistory( static_cast<uint64_steamid>( steamIDFriend ), static_cast<int>( iPersonaName ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendPersonaState( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendPersonaState( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendRelationship( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendRelationship( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetFriendRichPresence( uint64_t steamIDFriend, const char * pchKey )
{
	try
	{
		return Steam_Friends_GetFriendRichPresence( static_cast<uint64_steamid>( steamIDFriend ), pchKey );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex( uint64_t steamIDFriend, int32_t iKey )
{
	try
	{
		return Steam_Friends_GetFriendRichPresenceKeyByIndex( static_cast<uint64_steamid>( steamIDFriend ), static_cast<int>( iKey ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendRichPresenceKeyCount( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendSteamLevel( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendSteamLevel( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendsGroupCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendsGroupCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int16_t SWS_SteamAPI_ISteamFriends_GetFriendsGroupIDByIndex( int32_t iFG )
{
	try
	{
		return static_cast<int16_t>( Steam_Friends_GetFriendsGroupIDByIndex( static_cast<int>( iFG ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetFriendsGroupMembersCount( int16_t friendsGroupID )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendsGroupMembersCount( static_cast<FriendsGroupID_t>( friendsGroupID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetFriendsGroupName( int16_t friendsGroupID )
{
	try
	{
		return Steam_Friends_GetFriendsGroupName( static_cast<FriendsGroupID_t>( friendsGroupID ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetLargeFriendAvatar( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetLargeFriendAvatar( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetMediumFriendAvatar( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetMediumFriendAvatar( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetNumChatsWithUnreadPriorityMessages( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetNumChatsWithUnreadPriorityMessages() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetPersonaName( void )
{
	try
	{
		return Steam_Friends_GetPersonaName();
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetPersonaState( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetPersonaState() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetPlayerNickname( uint64_t steamIDPlayer )
{
	try
	{
		return Steam_Friends_GetPlayerNickname( static_cast<uint64_steamid>( steamIDPlayer ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamFriends_GetProfileItemPropertyString( uint64_t steamID, int32_t itemType, int32_t prop )
{
	try
	{
		return Steam_Friends_GetProfileItemPropertyString( static_cast<uint64_steamid>( steamID ), static_cast<ECommunityProfileItemType>( itemType ), static_cast<ECommunityProfileItemProperty>( prop ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

uint32_t SWS_SteamAPI_ISteamFriends_GetProfileItemPropertyUint( uint64_t steamID, int32_t itemType, int32_t prop )
{
	try
	{
		return static_cast<uint32_t>( Steam_Friends_GetProfileItemPropertyUint( static_cast<uint64_steamid>( steamID ), static_cast<ECommunityProfileItemType>( itemType ), static_cast<ECommunityProfileItemProperty>( prop ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamFriends_GetSmallFriendAvatar( uint64_t steamIDFriend )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetSmallFriendAvatar( static_cast<uint64_steamid>( steamIDFriend ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamFriends_HasFriend( uint64_t steamIDFriend, int32_t iFriendFlags )
{
	try
	{
		return Steam_Friends_HasFriend( static_cast<uint64_steamid>( steamIDFriend ), static_cast<int>( iFriendFlags ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_InviteUserToGame( uint64_t steamIDFriend, const char * pchConnectString )
{
	try
	{
		return Steam_Friends_InviteUserToGame( static_cast<uint64_steamid>( steamIDFriend ), pchConnectString );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_IsClanChatAdmin( uint64_t steamIDClanChat, uint64_t steamIDUser )
{
	try
	{
		return Steam_Friends_IsClanChatAdmin( static_cast<uint64_steamid>( steamIDClanChat ), static_cast<uint64_steamid>( steamIDUser ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_IsClanChatWindowOpenInSteam( uint64_t steamIDClanChat )
{
	try
	{
		return Steam_Friends_IsClanChatWindowOpenInSteam( static_cast<uint64_steamid>( steamIDClanChat ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_IsClanOfficialGameGroup( uint64_t steamIDClan )
{
	try
	{
		return Steam_Friends_IsClanOfficialGameGroup( static_cast<uint64_steamid>( steamIDClan ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_IsClanPublic( uint64_t steamIDClan )
{
	try
	{
		return Steam_Friends_IsClanPublic( static_cast<uint64_steamid>( steamIDClan ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_IsFollowing( uint64_t steamID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_IsFollowing( static_cast<uint64_steamid>( steamID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamFriends_IsUserInSource( uint64_t steamIDUser, uint64_t steamIDSource )
{
	try
	{
		return Steam_Friends_IsUserInSource( static_cast<uint64_steamid>( steamIDUser ), static_cast<uint64_steamid>( steamIDSource ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_JoinClanChatRoom( uint64_t steamIDClan )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_JoinClanChatRoom( static_cast<uint64_steamid>( steamIDClan ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamFriends_LeaveClanChatRoom( uint64_t steamIDClan )
{
	try
	{
		return Steam_Friends_LeaveClanChatRoom( static_cast<uint64_steamid>( steamIDClan ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_OpenClanChatWindowInSteam( uint64_t steamIDClanChat )
{
	try
	{
		return Steam_Friends_OpenClanChatWindowInSteam( static_cast<uint64_steamid>( steamIDClanChat ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_RegisterProtocolInOverlayBrowser( const char * pchProtocol )
{
	try
	{
		return Steam_Friends_RegisterProtocolInOverlayBrowser( pchProtocol );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_ReplyToFriendMessage( uint64_t steamIDFriend, const char * pchMsgToSend )
{
	try
	{
		return Steam_Friends_ReplyToFriendMessage( static_cast<uint64_steamid>( steamIDFriend ), pchMsgToSend );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_RequestClanOfficerList( uint64_t steamIDClan )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_RequestClanOfficerList( static_cast<uint64_steamid>( steamIDClan ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamFriends_RequestEquippedProfileItems( uint64_t steamID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_RequestEquippedProfileItems( static_cast<uint64_steamid>( steamID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamFriends_RequestFriendRichPresence( uint64_t steamIDFriend )
{
	try
	{
		Steam_Friends_RequestFriendRichPresence( static_cast<uint64_steamid>( steamIDFriend ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamFriends_RequestUserInformation( uint64_t steamIDUser, bool bRequireNameOnly )
{
	try
	{
		return Steam_Friends_RequestUserInformation( static_cast<uint64_steamid>( steamIDUser ), bRequireNameOnly );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamFriends_SendClanChatMessage( uint64_t steamIDClanChat, const char * pchText )
{
	try
	{
		return Steam_Friends_SendClanChatMessage( static_cast<uint64_steamid>( steamIDClanChat ), pchText );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamFriends_SetInGameVoiceSpeaking( uint64_t steamIDUser, bool bSpeaking )
{
	try
	{
		Steam_Friends_SetInGameVoiceSpeaking( static_cast<uint64_steamid>( steamIDUser ), bSpeaking );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamFriends_SetListenForFriendsMessages( bool bInterceptEnabled )
{
	try
	{
		return Steam_Friends_SetListenForFriendsMessages( bInterceptEnabled );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamFriends_SetPlayedWith( uint64_t steamIDUserPlayedWith )
{
	try
	{
		Steam_Friends_SetPlayedWith( static_cast<uint64_steamid>( steamIDUserPlayedWith ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamFriends_SetRichPresence( const char * pchKey, const char * pchValue )
{
	try
	{
		return Steam_Friends_SetRichPresence( pchKey, pchValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamGameServerStats_ClearUserAchievement( uint64_t steamIDUser, const char * pchName )
{
	try
	{
		return Steam_GameServerStats_ClearUserAchievement( static_cast<uint64_steamid>( steamIDUser ), pchName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamGameServerStats_RequestUserStats( uint64_t steamIDUser )
{
	try
	{
		return static_cast<uint64_t>( Steam_GameServerStats_RequestUserStats( static_cast<uint64_steamid>( steamIDUser ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamGameServerStats_SetUserAchievement( uint64_t steamIDUser, const char * pchName )
{
	try
	{
		return Steam_GameServerStats_SetUserAchievement( static_cast<uint64_steamid>( steamIDUser ), pchName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamGameServerStats_SetUserStatFloat( uint64_t steamIDUser, const char * pchName, float fData )
{
	try
	{
		return Steam_GameServerStats_SetUserStat( static_cast<uint64_steamid>( steamIDUser ), pchName, fData );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamGameServerStats_SetUserStatInt32( uint64_t steamIDUser, const char * pchName, int32_t nData )
{
	try
	{
		return Steam_GameServerStats_SetUserStat( static_cast<uint64_steamid>( steamIDUser ), pchName, static_cast<int32>( nData ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamGameServerStats_StoreUserStats( uint64_t steamIDUser )
{
	try
	{
		return static_cast<uint64_t>( Steam_GameServerStats_StoreUserStats( static_cast<uint64_steamid>( steamIDUser ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamGameServerStats_UpdateUserAvgRateStat( uint64_t steamIDUser, const char * pchName, float flCountThisSession, double dSessionLength )
{
	try
	{
		return Steam_GameServerStats_UpdateUserAvgRateStat( static_cast<uint64_steamid>( steamIDUser ), pchName, flCountThisSession, dSessionLength );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamGameServer_AssociateWithClan( uint64_t steamIDClan )
{
	try
	{
		return static_cast<uint64_t>( Steam_GameServer_AssociateWithClan( static_cast<uint64_steamid>( steamIDClan ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamGameServer_BLoggedOn( void )
{
	try
	{
		return Steam_GameServer_BLoggedOn();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamGameServer_BSecure( void )
{
	try
	{
		return Steam_GameServer_BSecure();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamGameServer_BUpdateUserData( uint64_t steamIDUser, const char * pchPlayerName, uint32_t uScore )
{
	try
	{
		return Steam_GameServer_BUpdateUserData( static_cast<uint64_steamid>( steamIDUser ), pchPlayerName, static_cast<uint32>( uScore ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamGameServer_CancelAuthTicket( uint32_t hAuthTicket )
{
	try
	{
		Steam_GameServer_CancelAuthTicket( static_cast<HAuthTicket>( hAuthTicket ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_ClearAllKeyValues( void )
{
	try
	{
		Steam_GameServer_ClearAllKeyValues();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamGameServer_ComputeNewPlayerCompatibility( uint64_t steamIDNewPlayer )
{
	try
	{
		return static_cast<uint64_t>( Steam_GameServer_ComputeNewPlayerCompatibility( static_cast<uint64_steamid>( steamIDNewPlayer ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamGameServer_CreateUnauthenticatedUserConnection( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_GameServer_CreateUnauthenticatedUserConnection() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamGameServer_EndAuthSession( uint64_t steamID )
{
	try
	{
		Steam_GameServer_EndAuthSession( static_cast<uint64_steamid>( steamID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_GetGameplayStats( void )
{
	try
	{
		Steam_GameServer_GetGameplayStats();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamGameServer_GetServerReputation( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_GameServer_GetServerReputation() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamGameServer_GetSteamID( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_GameServer_GetSteamID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamGameServer_LogOff( void )
{
	try
	{
		Steam_GameServer_LogOff();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_LogOn( const char * pszToken )
{
	try
	{
		Steam_GameServer_LogOn( pszToken );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_LogOnAnonymous( void )
{
	try
	{
		Steam_GameServer_LogOnAnonymous();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamGameServer_RequestUserGroupStatus( uint64_t steamIDUser, uint64_t steamIDGroup )
{
	try
	{
		return Steam_GameServer_RequestUserGroupStatus( static_cast<uint64_steamid>( steamIDUser ), static_cast<uint64_steamid>( steamIDGroup ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamGameServer_SendUserDisconnect_DEPRECATED( uint64_t steamIDUser )
{
	try
	{
		Steam_GameServer_SendUserDisconnect_DEPRECATED( static_cast<uint64_steamid>( steamIDUser ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetAdvertiseServerActive( bool bActive )
{
	try
	{
		Steam_GameServer_SetAdvertiseServerActive( bActive );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetBotPlayerCount( int32_t cBotplayers )
{
	try
	{
		Steam_GameServer_SetBotPlayerCount( static_cast<int>( cBotplayers ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetDedicatedServer( bool bDedicated )
{
	try
	{
		Steam_GameServer_SetDedicatedServer( bDedicated );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetGameData( const char * pchGameData )
{
	try
	{
		Steam_GameServer_SetGameData( pchGameData );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetGameDescription( const char * pszGameDescription )
{
	try
	{
		Steam_GameServer_SetGameDescription( pszGameDescription );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetGameTags( const char * pchGameTags )
{
	try
	{
		Steam_GameServer_SetGameTags( pchGameTags );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetKeyValue( const char * pKey, const char * pValue )
{
	try
	{
		Steam_GameServer_SetKeyValue( pKey, pValue );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetMapName( const char * pszMapName )
{
	try
	{
		Steam_GameServer_SetMapName( pszMapName );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetMaxPlayerCount( int32_t cPlayersMax )
{
	try
	{
		Steam_GameServer_SetMaxPlayerCount( static_cast<int>( cPlayersMax ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetModDir( const char * pszModDir )
{
	try
	{
		Steam_GameServer_SetModDir( pszModDir );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetPasswordProtected( bool bPasswordProtected )
{
	try
	{
		Steam_GameServer_SetPasswordProtected( bPasswordProtected );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetProduct( const char * pszProduct )
{
	try
	{
		Steam_GameServer_SetProduct( pszProduct );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetRegion( const char * pszRegion )
{
	try
	{
		Steam_GameServer_SetRegion( pszRegion );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetServerName( const char * pszServerName )
{
	try
	{
		Steam_GameServer_SetServerName( pszServerName );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetSpectatorPort( uint16_t unSpectatorPort )
{
	try
	{
		Steam_GameServer_SetSpectatorPort( static_cast<uint16>( unSpectatorPort ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamGameServer_SetSpectatorServerName( const char * pszSpectatorServerName )
{
	try
	{
		Steam_GameServer_SetSpectatorServerName( pszSpectatorServerName );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamGameServer_UserHasLicenseForApp( uint64_t steamID, uint32_t appID )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServer_UserHasLicenseForApp( static_cast<uint64_steamid>( steamID ), static_cast<AppId_t>( appID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamGameServer_WasRestartRequested( void )
{
	try
	{
		return Steam_GameServer_WasRestartRequested();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_AddHeader( uint32_t unBrowserHandle, const char * pchKey, const char * pchValue )
{
	try
	{
		Steam_HTMLSurface_AddHeader( static_cast<HHTMLBrowser>( unBrowserHandle ), pchKey, pchValue );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_AllowStartRequest( uint32_t unBrowserHandle, bool bAllowed )
{
	try
	{
		Steam_HTMLSurface_AllowStartRequest( static_cast<HHTMLBrowser>( unBrowserHandle ), bAllowed );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_CopyToClipboard( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_CopyToClipboard( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamHTMLSurface_CreateBrowser( const char * pchUserAgent, const char * pchUserCSS )
{
	try
	{
		return static_cast<uint64_t>( Steam_HTMLSurface_CreateBrowser( pchUserAgent, pchUserCSS ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_ExecuteJavascript( uint32_t unBrowserHandle, const char * pchScript )
{
	try
	{
		Steam_HTMLSurface_ExecuteJavascript( static_cast<HHTMLBrowser>( unBrowserHandle ), pchScript );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_Find( uint32_t unBrowserHandle, const char * pchSearchStr, bool bCurrentlyInFind, bool bReverse )
{
	try
	{
		Steam_HTMLSurface_Find( static_cast<HHTMLBrowser>( unBrowserHandle ), pchSearchStr, bCurrentlyInFind, bReverse );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_GetLinkAtPosition( uint32_t unBrowserHandle, int32_t x, int32_t y )
{
	try
	{
		Steam_HTMLSurface_GetLinkAtPosition( static_cast<HHTMLBrowser>( unBrowserHandle ), static_cast<int>( x ), static_cast<int>( y ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_GoBack( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_GoBack( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_GoForward( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_GoForward( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamHTMLSurface_Init( void )
{
	try
	{
		return Steam_HTMLSurface_Init();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_JSDialogResponse( uint32_t unBrowserHandle, bool bResult )
{
	try
	{
		Steam_HTMLSurface_JSDialogResponse( static_cast<HHTMLBrowser>( unBrowserHandle ), bResult );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_LoadURL( uint32_t unBrowserHandle, const char * pchURL, const char * pchPostData )
{
	try
	{
		Steam_HTMLSurface_LoadURL( static_cast<HHTMLBrowser>( unBrowserHandle ), pchURL, pchPostData );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_MouseMove( uint32_t unBrowserHandle, int32_t x, int32_t y )
{
	try
	{
		Steam_HTMLSurface_MouseMove( static_cast<HHTMLBrowser>( unBrowserHandle ), static_cast<int>( x ), static_cast<int>( y ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_MouseWheel( uint32_t unBrowserHandle, int32_t nDelta )
{
	try
	{
		Steam_HTMLSurface_MouseWheel( static_cast<HHTMLBrowser>( unBrowserHandle ), static_cast<int32>( nDelta ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_OpenDeveloperTools( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_OpenDeveloperTools( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_PasteFromClipboard( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_PasteFromClipboard( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_Reload( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_Reload( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_RemoveBrowser( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_RemoveBrowser( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_SetBackgroundMode( uint32_t unBrowserHandle, bool bBackgroundMode )
{
	try
	{
		Steam_HTMLSurface_SetBackgroundMode( static_cast<HHTMLBrowser>( unBrowserHandle ), bBackgroundMode );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_SetCookie( const char * pchHostname, const char * pchKey, const char * pchValue, const char * pchPath, uint32_t nExpires, bool bSecure, bool bHTTPOnly )
{
	try
	{
		Steam_HTMLSurface_SetCookie( pchHostname, pchKey, pchValue, pchPath, static_cast<RTime32>( nExpires ), bSecure, bHTTPOnly );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_SetDPIScalingFactor( uint32_t unBrowserHandle, float flDPIScaling )
{
	try
	{
		Steam_HTMLSurface_SetDPIScalingFactor( static_cast<HHTMLBrowser>( unBrowserHandle ), flDPIScaling );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_SetHorizontalScroll( uint32_t unBrowserHandle, uint32_t nAbsolutePixelScroll )
{
	try
	{
		Steam_HTMLSurface_SetHorizontalScroll( static_cast<HHTMLBrowser>( unBrowserHandle ), static_cast<uint32>( nAbsolutePixelScroll ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_SetKeyFocus( uint32_t unBrowserHandle, bool bHasKeyFocus )
{
	try
	{
		Steam_HTMLSurface_SetKeyFocus( static_cast<HHTMLBrowser>( unBrowserHandle ), bHasKeyFocus );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_SetPageScaleFactor( uint32_t unBrowserHandle, float flZoom, int32_t nPointX, int32_t nPointY )
{
	try
	{
		Steam_HTMLSurface_SetPageScaleFactor( static_cast<HHTMLBrowser>( unBrowserHandle ), flZoom, static_cast<int>( nPointX ), static_cast<int>( nPointY ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_SetSize( uint32_t unBrowserHandle, uint32_t unWidth, uint32_t unHeight )
{
	try
	{
		Steam_HTMLSurface_SetSize( static_cast<HHTMLBrowser>( unBrowserHandle ), static_cast<uint32>( unWidth ), static_cast<uint32>( unHeight ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_SetVerticalScroll( uint32_t unBrowserHandle, uint32_t nAbsolutePixelScroll )
{
	try
	{
		Steam_HTMLSurface_SetVerticalScroll( static_cast<HHTMLBrowser>( unBrowserHandle ), static_cast<uint32>( nAbsolutePixelScroll ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamHTMLSurface_Shutdown( void )
{
	try
	{
		return Steam_HTMLSurface_Shutdown();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_StopFind( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_StopFind( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_StopLoad( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_StopLoad( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamHTMLSurface_ViewSource( uint32_t unBrowserHandle )
{
	try
	{
		Steam_HTMLSurface_ViewSource( static_cast<HHTMLBrowser>( unBrowserHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint32_t SWS_SteamAPI_ISteamHTTP_CreateCookieContainer( bool bAllowResponsesToModify )
{
	try
	{
		return static_cast<uint32_t>( Steam_HTTP_CreateCookieContainer( bAllowResponsesToModify ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamHTTP_CreateHTTPRequest( int32_t eHTTPRequestMethod, const char * pchAbsoluteURL )
{
	try
	{
		return static_cast<uint32_t>( Steam_HTTP_CreateHTTPRequest( static_cast<EHTTPMethod>( eHTTPRequestMethod ), pchAbsoluteURL ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamHTTP_DeferHTTPRequest( uint32_t hRequest )
{
	try
	{
		return Steam_HTTP_DeferHTTPRequest( static_cast<HTTPRequestHandle>( hRequest ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_PrioritizeHTTPRequest( uint32_t hRequest )
{
	try
	{
		return Steam_HTTP_PrioritizeHTTPRequest( static_cast<HTTPRequestHandle>( hRequest ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_ReleaseCookieContainer( uint32_t hCookieContainer )
{
	try
	{
		return Steam_HTTP_ReleaseCookieContainer( static_cast<HTTPCookieContainerHandle>( hCookieContainer ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_ReleaseHTTPRequest( uint32_t hRequest )
{
	try
	{
		return Steam_HTTP_ReleaseHTTPRequest( static_cast<HTTPRequestHandle>( hRequest ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetCookie( uint32_t hCookieContainer, const char * pchHost, const char * pchUrl, const char * pchCookie )
{
	try
	{
		return Steam_HTTP_SetCookie( static_cast<HTTPCookieContainerHandle>( hCookieContainer ), pchHost, pchUrl, pchCookie );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetHTTPRequestAbsoluteTimeoutMS( uint32_t hRequest, uint32_t unMilliseconds )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestAbsoluteTimeoutMS( static_cast<HTTPRequestHandle>( hRequest ), static_cast<uint32>( unMilliseconds ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetHTTPRequestContextValue( uint32_t hRequest, uint64_t ulContextValue )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestContextValue( static_cast<HTTPRequestHandle>( hRequest ), static_cast<uint64>( ulContextValue ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetHTTPRequestCookieContainer( uint32_t hRequest, uint32_t hCookieContainer )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestCookieContainer( static_cast<HTTPRequestHandle>( hRequest ), static_cast<HTTPCookieContainerHandle>( hCookieContainer ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetHTTPRequestGetOrPostParameter( uint32_t hRequest, const char * pchParamName, const char * pchParamValue )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestGetOrPostParameter( static_cast<HTTPRequestHandle>( hRequest ), pchParamName, pchParamValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetHTTPRequestHeaderValue( uint32_t hRequest, const char * pchHeaderName, const char * pchHeaderValue )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestHeaderValue( static_cast<HTTPRequestHandle>( hRequest ), pchHeaderName, pchHeaderValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetHTTPRequestNetworkActivityTimeout( uint32_t hRequest, uint32_t unTimeoutSeconds )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestNetworkActivityTimeout( static_cast<HTTPRequestHandle>( hRequest ), static_cast<uint32>( unTimeoutSeconds ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetHTTPRequestRequiresVerifiedCertificate( uint32_t hRequest, bool bRequireVerifiedCertificate )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestRequiresVerifiedCertificate( static_cast<HTTPRequestHandle>( hRequest ), bRequireVerifiedCertificate );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamHTTP_SetHTTPRequestUserAgentInfo( uint32_t hRequest, const char * pchUserAgentInfo )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestUserAgentInfo( static_cast<HTTPRequestHandle>( hRequest ), pchUserAgentInfo );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamInput_ActivateActionSet( uint64_t inputHandle, uint64_t actionSetHandle )
{
	try
	{
		Steam_Input_ActivateActionSet( static_cast<InputHandle_t>( inputHandle ), static_cast<InputActionSetHandle_t>( actionSetHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamInput_ActivateActionSetLayer( uint64_t inputHandle, uint64_t actionSetLayerHandle )
{
	try
	{
		Steam_Input_ActivateActionSetLayer( static_cast<InputHandle_t>( inputHandle ), static_cast<InputActionSetHandle_t>( actionSetLayerHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamInput_BNewDataAvailable( void )
{
	try
	{
		return Steam_Input_BNewDataAvailable();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamInput_BWaitForData( bool bWaitForever, uint32_t unTimeout )
{
	try
	{
		return Steam_Input_BWaitForData( bWaitForever, static_cast<uint32>( unTimeout ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamInput_DeactivateActionSetLayer( uint64_t inputHandle, uint64_t actionSetLayerHandle )
{
	try
	{
		Steam_Input_DeactivateActionSetLayer( static_cast<InputHandle_t>( inputHandle ), static_cast<InputActionSetHandle_t>( actionSetLayerHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamInput_DeactivateAllActionSetLayers( uint64_t inputHandle )
{
	try
	{
		Steam_Input_DeactivateAllActionSetLayers( static_cast<InputHandle_t>( inputHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamInput_EnableDeviceCallbacks( void )
{
	try
	{
		Steam_Input_EnableDeviceCallbacks();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamInput_GetActionOriginFromXboxOrigin( uint64_t inputHandle, int32_t eOrigin )
{
	try
	{
		return static_cast<int32_t>( Steam_Input_GetActionOriginFromXboxOrigin( static_cast<InputHandle_t>( inputHandle ), static_cast<EXboxOrigin>( eOrigin ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamInput_GetActionSetHandle( const char * pszActionSetName )
{
	try
	{
		return static_cast<uint64_t>( Steam_Input_GetActionSetHandle( pszActionSetName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamInput_GetAnalogActionHandle( const char * pszActionName )
{
	try
	{
		return static_cast<uint64_t>( Steam_Input_GetAnalogActionHandle( pszActionName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamInput_GetControllerForGamepadIndex( int32_t nIndex )
{
	try
	{
		return static_cast<uint64_t>( Steam_Input_GetControllerForGamepadIndex( static_cast<int>( nIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamInput_GetCurrentActionSet( uint64_t inputHandle )
{
	try
	{
		return static_cast<uint64_t>( Steam_Input_GetCurrentActionSet( static_cast<InputHandle_t>( inputHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamInput_GetDigitalActionHandle( const char * pszActionName )
{
	try
	{
		return static_cast<uint64_t>( Steam_Input_GetDigitalActionHandle( pszActionName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamInput_GetGamepadIndexForController( uint64_t ulinputHandle )
{
	try
	{
		return static_cast<int32_t>( Steam_Input_GetGamepadIndexForController( static_cast<InputHandle_t>( ulinputHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamInput_GetGlyphForActionOrigin_Legacy( int32_t eOrigin )
{
	try
	{
		return Steam_Input_GetGlyphForActionOrigin_Legacy( static_cast<EInputActionOrigin>( eOrigin ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamInput_GetGlyphForXboxOrigin( int32_t eOrigin )
{
	try
	{
		return Steam_Input_GetGlyphForXboxOrigin( static_cast<EXboxOrigin>( eOrigin ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamInput_GetGlyphPNGForActionOrigin( int32_t eOrigin, int32_t eSize, uint32_t unFlags )
{
	try
	{
		return Steam_Input_GetGlyphPNGForActionOrigin( static_cast<EInputActionOrigin>( eOrigin ), static_cast<ESteamInputGlyphSize>( eSize ), static_cast<uint32>( unFlags ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamInput_GetGlyphSVGForActionOrigin( int32_t eOrigin, uint32_t unFlags )
{
	try
	{
		return Steam_Input_GetGlyphSVGForActionOrigin( static_cast<EInputActionOrigin>( eOrigin ), static_cast<uint32>( unFlags ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamInput_GetInputTypeForHandle( uint64_t inputHandle )
{
	try
	{
		return static_cast<int32_t>( Steam_Input_GetInputTypeForHandle( static_cast<InputHandle_t>( inputHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamInput_GetRemotePlaySessionID( uint64_t inputHandle )
{
	try
	{
		return static_cast<uint32_t>( Steam_Input_GetRemotePlaySessionID( static_cast<InputHandle_t>( inputHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint16_t SWS_SteamAPI_ISteamInput_GetSessionInputConfigurationSettings( void )
{
	try
	{
		return static_cast<uint16_t>( Steam_Input_GetSessionInputConfigurationSettings() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamInput_GetStringForActionOrigin( int32_t eOrigin )
{
	try
	{
		return Steam_Input_GetStringForActionOrigin( static_cast<EInputActionOrigin>( eOrigin ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamInput_GetStringForAnalogActionName( uint64_t eActionHandle )
{
	try
	{
		return Steam_Input_GetStringForAnalogActionName( static_cast<InputAnalogActionHandle_t>( eActionHandle ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamInput_GetStringForDigitalActionName( uint64_t eActionHandle )
{
	try
	{
		return Steam_Input_GetStringForDigitalActionName( static_cast<InputDigitalActionHandle_t>( eActionHandle ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

const char * SWS_SteamAPI_ISteamInput_GetStringForXboxOrigin( int32_t eOrigin )
{
	try
	{
		return Steam_Input_GetStringForXboxOrigin( static_cast<EXboxOrigin>( eOrigin ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

bool SWS_SteamAPI_ISteamInput_Init( bool bExplicitlyCallRunFrame )
{
	try
	{
		return Steam_Input_Init( bExplicitlyCallRunFrame );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamInput_Legacy_TriggerHapticPulse( uint64_t inputHandle, int32_t eTargetPad, uint16_t usDurationMicroSec )
{
	try
	{
		Steam_Input_Legacy_TriggerHapticPulse( static_cast<InputHandle_t>( inputHandle ), static_cast<ESteamControllerPad>( eTargetPad ), static_cast<unsigned short>( usDurationMicroSec ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamInput_Legacy_TriggerRepeatedHapticPulse( uint64_t inputHandle, int32_t eTargetPad, uint16_t usDurationMicroSec, uint16_t usOffMicroSec, uint16_t unRepeat, uint32_t nFlags )
{
	try
	{
		Steam_Input_Legacy_TriggerRepeatedHapticPulse( static_cast<InputHandle_t>( inputHandle ), static_cast<ESteamControllerPad>( eTargetPad ), static_cast<unsigned short>( usDurationMicroSec ), static_cast<unsigned short>( usOffMicroSec ), static_cast<unsigned short>( unRepeat ), static_cast<unsigned int>( nFlags ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamInput_RunFrame( bool bReservedValue )
{
	try
	{
		Steam_Input_RunFrame( bReservedValue );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamInput_SetInputActionManifestFilePath( const char * pchInputActionManifestAbsolutePath )
{
	try
	{
		return Steam_Input_SetInputActionManifestFilePath( pchInputActionManifestAbsolutePath );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamInput_SetLEDColor( uint64_t inputHandle, uint8_t nColorR, uint8_t nColorG, uint8_t nColorB, uint32_t nFlags )
{
	try
	{
		Steam_Input_SetLEDColor( static_cast<InputHandle_t>( inputHandle ), static_cast<uint8>( nColorR ), static_cast<uint8>( nColorG ), static_cast<uint8>( nColorB ), static_cast<unsigned int>( nFlags ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamInput_ShowBindingPanel( uint64_t inputHandle )
{
	try
	{
		return Steam_Input_ShowBindingPanel( static_cast<InputHandle_t>( inputHandle ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamInput_Shutdown( void )
{
	try
	{
		return Steam_Input_Shutdown();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamInput_StopAnalogActionMomentum( uint64_t inputHandle, uint64_t eAction )
{
	try
	{
		Steam_Input_StopAnalogActionMomentum( static_cast<InputHandle_t>( inputHandle ), static_cast<InputAnalogActionHandle_t>( eAction ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamInput_TranslateActionOrigin( int32_t eDestinationInputType, int32_t eSourceOrigin )
{
	try
	{
		return static_cast<int32_t>( Steam_Input_TranslateActionOrigin( static_cast<ESteamInputType>( eDestinationInputType ), static_cast<EInputActionOrigin>( eSourceOrigin ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamInput_TriggerSimpleHapticEvent( uint64_t inputHandle, int32_t eHapticLocation, uint8_t nIntensity, char nGainDB, uint8_t nOtherIntensity, char nOtherGainDB )
{
	try
	{
		Steam_Input_TriggerSimpleHapticEvent( static_cast<InputHandle_t>( inputHandle ), static_cast<EControllerHapticLocation>( eHapticLocation ), static_cast<uint8>( nIntensity ), nGainDB, static_cast<uint8>( nOtherIntensity ), nOtherGainDB );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamInput_TriggerVibration( uint64_t inputHandle, uint16_t usLeftSpeed, uint16_t usRightSpeed )
{
	try
	{
		Steam_Input_TriggerVibration( static_cast<InputHandle_t>( inputHandle ), static_cast<unsigned short>( usLeftSpeed ), static_cast<unsigned short>( usRightSpeed ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamInput_TriggerVibrationExtended( uint64_t inputHandle, uint16_t usLeftSpeed, uint16_t usRightSpeed, uint16_t usLeftTriggerSpeed, uint16_t usRightTriggerSpeed )
{
	try
	{
		Steam_Input_TriggerVibrationExtended( static_cast<InputHandle_t>( inputHandle ), static_cast<unsigned short>( usLeftSpeed ), static_cast<unsigned short>( usRightSpeed ), static_cast<unsigned short>( usLeftTriggerSpeed ), static_cast<unsigned short>( usRightTriggerSpeed ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamInventory_CheckResultSteamID( int32_t resultHandle, uint64_t steamIDExpected )
{
	try
	{
		return Steam_Inventory_CheckResultSteamID( static_cast<SteamInventoryResult_t>( resultHandle ), static_cast<uint64_steamid>( steamIDExpected ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamInventory_DestroyResult( int32_t resultHandle )
{
	try
	{
		Steam_Inventory_DestroyResult( static_cast<SteamInventoryResult_t>( resultHandle ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint32_t SWS_SteamAPI_ISteamInventory_GetNumItemsWithPrices( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Inventory_GetNumItemsWithPrices() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamInventory_GetResultStatus( int32_t resultHandle )
{
	try
	{
		return static_cast<int32_t>( Steam_Inventory_GetResultStatus( static_cast<SteamInventoryResult_t>( resultHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamInventory_GetResultTimestamp( int32_t resultHandle )
{
	try
	{
		return static_cast<uint32_t>( Steam_Inventory_GetResultTimestamp( static_cast<SteamInventoryResult_t>( resultHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamInventory_LoadItemDefinitions( void )
{
	try
	{
		return Steam_Inventory_LoadItemDefinitions();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamInventory_RemoveProperty( uint64_t handle, uint64_t nItemID, const char * pchPropertyName )
{
	try
	{
		return Steam_Inventory_RemoveProperty( static_cast<SteamInventoryUpdateHandle_t>( handle ), static_cast<SteamItemInstanceID_t>( nItemID ), pchPropertyName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamInventory_RequestEligiblePromoItemDefinitionsIDs( uint64_t steamID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Inventory_RequestEligiblePromoItemDefinitionsIDs( static_cast<uint64_steamid>( steamID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamInventory_RequestPrices( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_Inventory_RequestPrices() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamInventory_SendItemDropHeartbeat( void )
{
	try
	{
		Steam_Inventory_SendItemDropHeartbeat();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamInventory_SetPropertyBool( uint64_t handle, uint64_t nItemID, const char * pchPropertyName, bool bValue )
{
	try
	{
		return Steam_Inventory_SetProperty( static_cast<SteamInventoryUpdateHandle_t>( handle ), static_cast<SteamItemInstanceID_t>( nItemID ), pchPropertyName, bValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamInventory_SetPropertyFloat( uint64_t handle, uint64_t nItemID, const char * pchPropertyName, float flValue )
{
	try
	{
		return Steam_Inventory_SetProperty( static_cast<SteamInventoryUpdateHandle_t>( handle ), static_cast<SteamItemInstanceID_t>( nItemID ), pchPropertyName, flValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamInventory_SetPropertyInt64( uint64_t handle, uint64_t nItemID, const char * pchPropertyName, int64_t nValue )
{
	try
	{
		return Steam_Inventory_SetProperty( static_cast<SteamInventoryUpdateHandle_t>( handle ), static_cast<SteamItemInstanceID_t>( nItemID ), pchPropertyName, static_cast<int64>( nValue ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamInventory_SetPropertyString( uint64_t handle, uint64_t nItemID, const char * pchPropertyName, const char * pchPropertyValue )
{
	try
	{
		return Steam_Inventory_SetProperty( static_cast<SteamInventoryUpdateHandle_t>( handle ), static_cast<SteamItemInstanceID_t>( nItemID ), pchPropertyName, pchPropertyValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamInventory_StartUpdateProperties( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_Inventory_StartUpdateProperties() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamMatchmakingServers_CancelServerQuery( int32_t hServerQuery )
{
	try
	{
		Steam_MatchmakingServers_CancelServerQuery( static_cast<HServerQuery>( hServerQuery ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamMatchmaking_AddFavoriteGame( uint32_t nAppID, uint32_t nIP, uint16_t nConnPort, uint16_t nQueryPort, uint32_t unFlags, uint32_t rTime32LastPlayedOnServer )
{
	try
	{
		return static_cast<int32_t>( Steam_Matchmaking_AddFavoriteGame( static_cast<AppId_t>( nAppID ), static_cast<uint32>( nIP ), static_cast<uint16>( nConnPort ), static_cast<uint16>( nQueryPort ), static_cast<uint32>( unFlags ), static_cast<uint32>( rTime32LastPlayedOnServer ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamMatchmaking_AddRequestLobbyListCompatibleMembersFilter( uint64_t steamIDLobby )
{
	try
	{
		Steam_Matchmaking_AddRequestLobbyListCompatibleMembersFilter( static_cast<uint64_steamid>( steamIDLobby ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter( int32_t eLobbyDistanceFilter )
{
	try
	{
		Steam_Matchmaking_AddRequestLobbyListDistanceFilter( static_cast<ELobbyDistanceFilter>( eLobbyDistanceFilter ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable( int32_t nSlotsAvailable )
{
	try
	{
		Steam_Matchmaking_AddRequestLobbyListFilterSlotsAvailable( static_cast<int>( nSlotsAvailable ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter( const char * pchKeyToMatch, int32_t nValueToBeCloseTo )
{
	try
	{
		Steam_Matchmaking_AddRequestLobbyListNearValueFilter( pchKeyToMatch, static_cast<int>( nValueToBeCloseTo ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter( const char * pchKeyToMatch, int32_t nValueToMatch, int32_t eComparisonType )
{
	try
	{
		Steam_Matchmaking_AddRequestLobbyListNumericalFilter( pchKeyToMatch, static_cast<int>( nValueToMatch ), static_cast<ELobbyComparison>( eComparisonType ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter( int32_t cMaxResults )
{
	try
	{
		Steam_Matchmaking_AddRequestLobbyListResultCountFilter( static_cast<int>( cMaxResults ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter( const char * pchKeyToMatch, const char * pchValueToMatch, int32_t eComparisonType )
{
	try
	{
		Steam_Matchmaking_AddRequestLobbyListStringFilter( pchKeyToMatch, pchValueToMatch, static_cast<ELobbyComparison>( eComparisonType ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamMatchmaking_CreateLobby( int32_t eLobbyType, int32_t cMaxMembers )
{
	try
	{
		return static_cast<uint64_t>( Steam_Matchmaking_CreateLobby( static_cast<ELobbyType>( eLobbyType ), static_cast<int>( cMaxMembers ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_DeleteLobbyData( uint64_t steamIDLobby, const char * pchKey )
{
	try
	{
		return Steam_Matchmaking_DeleteLobbyData( static_cast<uint64_steamid>( steamIDLobby ), pchKey );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamMatchmaking_GetFavoriteGameCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Matchmaking_GetFavoriteGameCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamMatchmaking_GetLobbyByIndex( int32_t iLobby )
{
	try
	{
		return static_cast<uint64_t>( Steam_Matchmaking_GetLobbyByIndex( static_cast<int>( iLobby ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamMatchmaking_GetLobbyData( uint64_t steamIDLobby, const char * pchKey )
{
	try
	{
		return Steam_Matchmaking_GetLobbyData( static_cast<uint64_steamid>( steamIDLobby ), pchKey );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamMatchmaking_GetLobbyDataCount( uint64_t steamIDLobby )
{
	try
	{
		return static_cast<int32_t>( Steam_Matchmaking_GetLobbyDataCount( static_cast<uint64_steamid>( steamIDLobby ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex( uint64_t steamIDLobby, int32_t iMember )
{
	try
	{
		return static_cast<uint64_t>( Steam_Matchmaking_GetLobbyMemberByIndex( static_cast<uint64_steamid>( steamIDLobby ), static_cast<int>( iMember ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamMatchmaking_GetLobbyMemberData( uint64_t steamIDLobby, uint64_t steamIDUser, const char * pchKey )
{
	try
	{
		return Steam_Matchmaking_GetLobbyMemberData( static_cast<uint64_steamid>( steamIDLobby ), static_cast<uint64_steamid>( steamIDUser ), pchKey );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamMatchmaking_GetLobbyMemberLimit( uint64_t steamIDLobby )
{
	try
	{
		return static_cast<int32_t>( Steam_Matchmaking_GetLobbyMemberLimit( static_cast<uint64_steamid>( steamIDLobby ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamMatchmaking_GetLobbyOwner( uint64_t steamIDLobby )
{
	try
	{
		return static_cast<uint64_t>( Steam_Matchmaking_GetLobbyOwner( static_cast<uint64_steamid>( steamIDLobby ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamMatchmaking_GetNumLobbyMembers( uint64_t steamIDLobby )
{
	try
	{
		return static_cast<int32_t>( Steam_Matchmaking_GetNumLobbyMembers( static_cast<uint64_steamid>( steamIDLobby ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_InviteUserToLobby( uint64_t steamIDLobby, uint64_t steamIDInvitee )
{
	try
	{
		return Steam_Matchmaking_InviteUserToLobby( static_cast<uint64_steamid>( steamIDLobby ), static_cast<uint64_steamid>( steamIDInvitee ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamMatchmaking_JoinLobby( uint64_t steamIDLobby )
{
	try
	{
		return static_cast<uint64_t>( Steam_Matchmaking_JoinLobby( static_cast<uint64_steamid>( steamIDLobby ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamMatchmaking_LeaveLobby( uint64_t steamIDLobby )
{
	try
	{
		Steam_Matchmaking_LeaveLobby( static_cast<uint64_steamid>( steamIDLobby ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_RemoveFavoriteGame( uint32_t nAppID, uint32_t nIP, uint16_t nConnPort, uint16_t nQueryPort, uint32_t unFlags )
{
	try
	{
		return Steam_Matchmaking_RemoveFavoriteGame( static_cast<AppId_t>( nAppID ), static_cast<uint32>( nIP ), static_cast<uint16>( nConnPort ), static_cast<uint16>( nQueryPort ), static_cast<uint32>( unFlags ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_RequestLobbyData( uint64_t steamIDLobby )
{
	try
	{
		return Steam_Matchmaking_RequestLobbyData( static_cast<uint64_steamid>( steamIDLobby ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamMatchmaking_RequestLobbyList( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_Matchmaking_RequestLobbyList() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_SetLinkedLobby( uint64_t steamIDLobby, uint64_t steamIDLobbyDependent )
{
	try
	{
		return Steam_Matchmaking_SetLinkedLobby( static_cast<uint64_steamid>( steamIDLobby ), static_cast<uint64_steamid>( steamIDLobbyDependent ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_SetLobbyData( uint64_t steamIDLobby, const char * pchKey, const char * pchValue )
{
	try
	{
		return Steam_Matchmaking_SetLobbyData( static_cast<uint64_steamid>( steamIDLobby ), pchKey, pchValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamMatchmaking_SetLobbyGameServer( uint64_t steamIDLobby, uint32_t unGameServerIP, uint16_t unGameServerPort, uint64_t steamIDGameServer )
{
	try
	{
		Steam_Matchmaking_SetLobbyGameServer( static_cast<uint64_steamid>( steamIDLobby ), static_cast<uint32>( unGameServerIP ), static_cast<uint16>( unGameServerPort ), static_cast<uint64_steamid>( steamIDGameServer ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_SetLobbyJoinable( uint64_t steamIDLobby, bool bLobbyJoinable )
{
	try
	{
		return Steam_Matchmaking_SetLobbyJoinable( static_cast<uint64_steamid>( steamIDLobby ), bLobbyJoinable );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamMatchmaking_SetLobbyMemberData( uint64_t steamIDLobby, const char * pchKey, const char * pchValue )
{
	try
	{
		Steam_Matchmaking_SetLobbyMemberData( static_cast<uint64_steamid>( steamIDLobby ), pchKey, pchValue );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_SetLobbyMemberLimit( uint64_t steamIDLobby, int32_t cMaxMembers )
{
	try
	{
		return Steam_Matchmaking_SetLobbyMemberLimit( static_cast<uint64_steamid>( steamIDLobby ), static_cast<int>( cMaxMembers ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_SetLobbyOwner( uint64_t steamIDLobby, uint64_t steamIDNewOwner )
{
	try
	{
		return Steam_Matchmaking_SetLobbyOwner( static_cast<uint64_steamid>( steamIDLobby ), static_cast<uint64_steamid>( steamIDNewOwner ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamMatchmaking_SetLobbyType( uint64_t steamIDLobby, int32_t eLobbyType )
{
	try
	{
		return Steam_Matchmaking_SetLobbyType( static_cast<uint64_steamid>( steamIDLobby ), static_cast<ELobbyType>( eLobbyType ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamMusic_BIsEnabled( void )
{
	try
	{
		return Steam_Music_BIsEnabled();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamMusic_BIsPlaying( void )
{
	try
	{
		return Steam_Music_BIsPlaying();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamMusic_GetPlaybackStatus( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Music_GetPlaybackStatus() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

float SWS_SteamAPI_ISteamMusic_GetVolume( void )
{
	try
	{
		return Steam_Music_GetVolume();
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamMusic_Pause( void )
{
	try
	{
		Steam_Music_Pause();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMusic_Play( void )
{
	try
	{
		Steam_Music_Play();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMusic_PlayNext( void )
{
	try
	{
		Steam_Music_PlayNext();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMusic_PlayPrevious( void )
{
	try
	{
		Steam_Music_PlayPrevious();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamMusic_SetVolume( float flVolume )
{
	try
	{
		Steam_Music_SetVolume( flVolume );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamNetworkingSockets_AcceptConnection( uint32_t hConn )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSockets_AcceptConnection( static_cast<HSteamNetConnection>( hConn ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamNetworkingSockets_BeginAsyncRequestFakeIP( int32_t nNumPorts )
{
	try
	{
		return Steam_NetworkingSockets_BeginAsyncRequestFakeIP( static_cast<int>( nNumPorts ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingSockets_CloseConnection( uint32_t hPeer, int32_t nReason, const char * pszDebug, bool bEnableLinger )
{
	try
	{
		return Steam_NetworkingSockets_CloseConnection( static_cast<HSteamNetConnection>( hPeer ), static_cast<int>( nReason ), pszDebug, bEnableLinger );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingSockets_CloseListenSocket( uint32_t hSocket )
{
	try
	{
		return Steam_NetworkingSockets_CloseListenSocket( static_cast<HSteamListenSocket>( hSocket ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint32_t SWS_SteamAPI_ISteamNetworkingSockets_CreatePollGroup( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_NetworkingSockets_CreatePollGroup() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamNetworkingSockets_DestroyPollGroup( uint32_t hPollGroup )
{
	try
	{
		return Steam_NetworkingSockets_DestroyPollGroup( static_cast<HSteamNetPollGroup>( hPollGroup ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamNetworkingSockets_FlushMessagesOnConnection( uint32_t hConn )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSockets_FlushMessagesOnConnection( static_cast<HSteamNetConnection>( hConn ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int64_t SWS_SteamAPI_ISteamNetworkingSockets_GetConnectionUserData( uint32_t hPeer )
{
	try
	{
		return static_cast<int64_t>( Steam_NetworkingSockets_GetConnectionUserData( static_cast<HSteamNetConnection>( hPeer ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPOPID( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_NetworkingSockets_GetHostedDedicatedServerPOPID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint16_t SWS_SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPort( void )
{
	try
	{
		return static_cast<uint16_t>( Steam_NetworkingSockets_GetHostedDedicatedServerPort() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamNetworkingSockets_InitAuthentication( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSockets_InitAuthentication() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamNetworkingSockets_RunCallbacks( void )
{
	try
	{
		Steam_NetworkingSockets_RunCallbacks();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamNetworkingSockets_SetConnectionName( uint32_t hPeer, const char * pszName )
{
	try
	{
		Steam_NetworkingSockets_SetConnectionName( static_cast<HSteamNetConnection>( hPeer ), pszName );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamNetworkingSockets_SetConnectionPollGroup( uint32_t hConn, uint32_t hPollGroup )
{
	try
	{
		return Steam_NetworkingSockets_SetConnectionPollGroup( static_cast<HSteamNetConnection>( hConn ), static_cast<HSteamNetPollGroup>( hPollGroup ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingSockets_SetConnectionUserData( uint32_t hPeer, int64_t nUserData )
{
	try
	{
		return Steam_NetworkingSockets_SetConnectionUserData( static_cast<HSteamNetConnection>( hPeer ), static_cast<int64>( nUserData ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingUtils_CheckPingDataUpToDate( float flMaxAgeSeconds )
{
	try
	{
		return Steam_NetworkingUtils_CheckPingDataUpToDate( flMaxAgeSeconds );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamNetworkingUtils_GetDirectPingToPOP( uint32_t popID )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingUtils_GetDirectPingToPOP( static_cast<SteamNetworkingPOPID>( popID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamNetworkingUtils_GetIPv4FakeIPType( uint32_t nIPv4 )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingUtils_GetIPv4FakeIPType( static_cast<uint32>( nIPv4 ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int64_t SWS_SteamAPI_ISteamNetworkingUtils_GetLocalTimestamp( void )
{
	try
	{
		return static_cast<int64_t>( Steam_NetworkingUtils_GetLocalTimestamp() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamNetworkingUtils_GetPOPCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingUtils_GetPOPCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess( void )
{
	try
	{
		Steam_NetworkingUtils_InitRelayNetworkAccess();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamNetworkingUtils_IsFakeIPv4( uint32_t nIPv4 )
{
	try
	{
		return Steam_NetworkingUtils_IsFakeIPv4( static_cast<uint32>( nIPv4 ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamNetworkingUtils_IterateGenericEditableConfigValues( int32_t eCurrent, bool bEnumerateDevVars )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingUtils_IterateGenericEditableConfigValues( static_cast<ESteamNetworkingConfigValue>( eCurrent ), bEnumerateDevVars ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueFloat( uint32_t hConn, int32_t eValue, float val )
{
	try
	{
		return Steam_NetworkingUtils_SetConnectionConfigValueFloat( static_cast<HSteamNetConnection>( hConn ), static_cast<ESteamNetworkingConfigValue>( eValue ), val );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueInt32( uint32_t hConn, int32_t eValue, int32_t val )
{
	try
	{
		return Steam_NetworkingUtils_SetConnectionConfigValueInt32( static_cast<HSteamNetConnection>( hConn ), static_cast<ESteamNetworkingConfigValue>( eValue ), static_cast<int32>( val ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueString( uint32_t hConn, int32_t eValue, const char * val )
{
	try
	{
		return Steam_NetworkingUtils_SetConnectionConfigValueString( static_cast<HSteamNetConnection>( hConn ), static_cast<ESteamNetworkingConfigValue>( eValue ), val );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueFloat( int32_t eValue, float val )
{
	try
	{
		return Steam_NetworkingUtils_SetGlobalConfigValueFloat( static_cast<ESteamNetworkingConfigValue>( eValue ), val );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueInt32( int32_t eValue, int32_t val )
{
	try
	{
		return Steam_NetworkingUtils_SetGlobalConfigValueInt32( static_cast<ESteamNetworkingConfigValue>( eValue ), static_cast<int32>( val ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueString( int32_t eValue, const char * val )
{
	try
	{
		return Steam_NetworkingUtils_SetGlobalConfigValueString( static_cast<ESteamNetworkingConfigValue>( eValue ), val );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser( uint64_t steamIDRemote )
{
	try
	{
		return Steam_Networking_AcceptP2PSessionWithUser( static_cast<uint64_steamid>( steamIDRemote ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworking_AllowP2PPacketRelay( bool bAllow )
{
	try
	{
		return Steam_Networking_AllowP2PPacketRelay( bAllow );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworking_CloseP2PChannelWithUser( uint64_t steamIDRemote, int32_t nChannel )
{
	try
	{
		return Steam_Networking_CloseP2PChannelWithUser( static_cast<uint64_steamid>( steamIDRemote ), static_cast<int>( nChannel ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworking_CloseP2PSessionWithUser( uint64_t steamIDRemote )
{
	try
	{
		return Steam_Networking_CloseP2PSessionWithUser( static_cast<uint64_steamid>( steamIDRemote ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint32_t SWS_SteamAPI_ISteamNetworking_CreateP2PConnectionSocket( uint64_t steamIDTarget, int32_t nVirtualPort, int32_t nTimeoutSec, bool bAllowUseOfPacketRelay )
{
	try
	{
		return static_cast<uint32_t>( Steam_Networking_CreateP2PConnectionSocket( static_cast<uint64_steamid>( steamIDTarget ), static_cast<int>( nVirtualPort ), static_cast<int>( nTimeoutSec ), bAllowUseOfPacketRelay ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamNetworking_DestroyListenSocket( uint32_t hSocket, bool bNotifyRemoteEnd )
{
	try
	{
		return Steam_Networking_DestroyListenSocket( static_cast<SNetListenSocket_t>( hSocket ), bNotifyRemoteEnd );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamNetworking_DestroySocket( uint32_t hSocket, bool bNotifyRemoteEnd )
{
	try
	{
		return Steam_Networking_DestroySocket( static_cast<SNetSocket_t>( hSocket ), bNotifyRemoteEnd );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamNetworking_GetMaxPacketSize( uint32_t hSocket )
{
	try
	{
		return static_cast<int32_t>( Steam_Networking_GetMaxPacketSize( static_cast<SNetSocket_t>( hSocket ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamNetworking_GetSocketConnectionType( uint32_t hSocket )
{
	try
	{
		return static_cast<int32_t>( Steam_Networking_GetSocketConnectionType( static_cast<SNetSocket_t>( hSocket ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamParentalSettings_BIsAppBlocked( uint32_t nAppID )
{
	try
	{
		return Steam_ParentalSettings_BIsAppBlocked( static_cast<AppId_t>( nAppID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamParentalSettings_BIsAppInBlockList( uint32_t nAppID )
{
	try
	{
		return Steam_ParentalSettings_BIsAppInBlockList( static_cast<AppId_t>( nAppID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamParentalSettings_BIsFeatureBlocked( int32_t eFeature )
{
	try
	{
		return Steam_ParentalSettings_BIsFeatureBlocked( static_cast<EParentalFeature>( eFeature ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamParentalSettings_BIsFeatureInBlockList( int32_t eFeature )
{
	try
	{
		return Steam_ParentalSettings_BIsFeatureInBlockList( static_cast<EParentalFeature>( eFeature ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamParentalSettings_BIsParentalLockEnabled( void )
{
	try
	{
		return Steam_ParentalSettings_BIsParentalLockEnabled();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamParentalSettings_BIsParentalLockLocked( void )
{
	try
	{
		return Steam_ParentalSettings_BIsParentalLockLocked();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamParties_CancelReservation( uint64_t ulBeacon, uint64_t steamIDUser )
{
	try
	{
		Steam_Parties_CancelReservation( static_cast<PartyBeaconID_t>( ulBeacon ), static_cast<uint64_steamid>( steamIDUser ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamParties_ChangeNumOpenSlots( uint64_t ulBeacon, uint32_t unOpenSlots )
{
	try
	{
		return static_cast<uint64_t>( Steam_Parties_ChangeNumOpenSlots( static_cast<PartyBeaconID_t>( ulBeacon ), static_cast<uint32>( unOpenSlots ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamParties_DestroyBeacon( uint64_t ulBeacon )
{
	try
	{
		return Steam_Parties_DestroyBeacon( static_cast<PartyBeaconID_t>( ulBeacon ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamParties_GetBeaconByIndex( uint32_t unIndex )
{
	try
	{
		return static_cast<uint64_t>( Steam_Parties_GetBeaconByIndex( static_cast<uint32>( unIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamParties_GetNumActiveBeacons( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Parties_GetNumActiveBeacons() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamParties_JoinParty( uint64_t ulBeaconID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Parties_JoinParty( static_cast<PartyBeaconID_t>( ulBeaconID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamParties_OnReservationCompleted( uint64_t ulBeacon, uint64_t steamIDUser )
{
	try
	{
		Steam_Parties_OnReservationCompleted( static_cast<PartyBeaconID_t>( ulBeacon ), static_cast<uint64_steamid>( steamIDUser ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamRemotePlay_BEnableRemotePlayTogetherDirectInput( void )
{
	try
	{
		return Steam_RemotePlay_BEnableRemotePlayTogetherDirectInput();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemotePlay_BSendRemotePlayTogetherInvite( uint64_t steamIDFriend )
{
	try
	{
		return Steam_RemotePlay_BSendRemotePlayTogetherInvite( static_cast<uint64_steamid>( steamIDFriend ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemotePlay_BSessionRemotePlayTogether( uint32_t unSessionID )
{
	try
	{
		return Steam_RemotePlay_BSessionRemotePlayTogether( static_cast<RemotePlaySessionID_t>( unSessionID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamRemotePlay_DisableRemotePlayTogetherDirectInput( void )
{
	try
	{
		Steam_RemotePlay_DisableRemotePlayTogetherDirectInput();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamRemotePlay_GetLargeSessionAvatar( uint32_t unSessionID )
{
	try
	{
		return static_cast<int32_t>( Steam_RemotePlay_GetLargeSessionAvatar( static_cast<RemotePlaySessionID_t>( unSessionID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamRemotePlay_GetMediumSessionAvatar( uint32_t unSessionID )
{
	try
	{
		return static_cast<int32_t>( Steam_RemotePlay_GetMediumSessionAvatar( static_cast<RemotePlaySessionID_t>( unSessionID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamRemotePlay_GetSessionClientFormFactor( uint32_t unSessionID )
{
	try
	{
		return static_cast<int32_t>( Steam_RemotePlay_GetSessionClientFormFactor( static_cast<RemotePlaySessionID_t>( unSessionID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamRemotePlay_GetSessionClientName( uint32_t unSessionID )
{
	try
	{
		return Steam_RemotePlay_GetSessionClientName( static_cast<RemotePlaySessionID_t>( unSessionID ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

uint32_t SWS_SteamAPI_ISteamRemotePlay_GetSessionCount( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_RemotePlay_GetSessionCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamRemotePlay_GetSessionGuestID( uint32_t unSessionID )
{
	try
	{
		return static_cast<uint32_t>( Steam_RemotePlay_GetSessionGuestID( static_cast<RemotePlaySessionID_t>( unSessionID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamRemotePlay_GetSessionID( int32_t iSessionIndex )
{
	try
	{
		return static_cast<uint32_t>( Steam_RemotePlay_GetSessionID( static_cast<int>( iSessionIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemotePlay_GetSessionSteamID( uint32_t unSessionID )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemotePlay_GetSessionSteamID( static_cast<RemotePlaySessionID_t>( unSessionID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamRemotePlay_GetSmallSessionAvatar( uint32_t unSessionID )
{
	try
	{
		return static_cast<int32_t>( Steam_RemotePlay_GetSmallSessionAvatar( static_cast<RemotePlaySessionID_t>( unSessionID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamRemotePlay_SetMouseCursor( uint32_t unSessionID, uint32_t unCursorID )
{
	try
	{
		Steam_RemotePlay_SetMouseCursor( static_cast<RemotePlaySessionID_t>( unSessionID ), static_cast<RemotePlayCursorID_t>( unCursorID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamRemotePlay_SetMousePosition( uint32_t unSessionID, float flNormalizedX, float flNormalizedY )
{
	try
	{
		Steam_RemotePlay_SetMousePosition( static_cast<RemotePlaySessionID_t>( unSessionID ), flNormalizedX, flNormalizedY );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamRemotePlay_SetMouseVisibility( uint32_t unSessionID, bool bVisible )
{
	try
	{
		Steam_RemotePlay_SetMouseVisibility( static_cast<RemotePlaySessionID_t>( unSessionID ), bVisible );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamRemotePlay_ShowRemotePlayTogetherUI( void )
{
	try
	{
		return Steam_RemotePlay_ShowRemotePlayTogetherUI();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_BeginFileWriteBatch( void )
{
	try
	{
		return Steam_RemoteStorage_BeginFileWriteBatch();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_CommitPublishedFileUpdate( uint64_t updateHandle )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_CommitPublishedFileUpdate( static_cast<PublishedFileUpdateHandle_t>( updateHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_CreatePublishedFileUpdateRequest( uint64_t unPublishedFileId )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_CreatePublishedFileUpdateRequest( static_cast<PublishedFileId_t>( unPublishedFileId ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_DeletePublishedFile( uint64_t unPublishedFileId )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_DeletePublishedFile( static_cast<PublishedFileId_t>( unPublishedFileId ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_EndFileWriteBatch( void )
{
	try
	{
		return Steam_RemoteStorage_EndFileWriteBatch();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_EnumeratePublishedFilesByUserAction( int32_t eAction, uint32_t unStartIndex )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_EnumeratePublishedFilesByUserAction( static_cast<EWorkshopFileAction>( eAction ), static_cast<uint32>( unStartIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_EnumerateUserPublishedFiles( uint32_t unStartIndex )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_EnumerateUserPublishedFiles( static_cast<uint32>( unStartIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_EnumerateUserSubscribedFiles( uint32_t unStartIndex )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_EnumerateUserSubscribedFiles( static_cast<uint32>( unStartIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_FileDelete( const char * pchFile )
{
	try
	{
		return Steam_RemoteStorage_FileDelete( pchFile );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_FileExists( const char * pchFile )
{
	try
	{
		return Steam_RemoteStorage_FileExists( pchFile );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_FileForget( const char * pchFile )
{
	try
	{
		return Steam_RemoteStorage_FileForget( pchFile );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_FilePersisted( const char * pchFile )
{
	try
	{
		return Steam_RemoteStorage_FilePersisted( pchFile );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_FileReadAsync( const char * pchFile, uint32_t nOffset, uint32_t cubToRead )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_FileReadAsync( pchFile, static_cast<uint32>( nOffset ), static_cast<uint32>( cubToRead ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_FileShare( const char * pchFile )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_FileShare( pchFile ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_FileWriteStreamCancel( uint64_t writeHandle )
{
	try
	{
		return Steam_RemoteStorage_FileWriteStreamCancel( static_cast<UGCFileWriteStreamHandle_t>( writeHandle ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_FileWriteStreamClose( uint64_t writeHandle )
{
	try
	{
		return Steam_RemoteStorage_FileWriteStreamClose( static_cast<UGCFileWriteStreamHandle_t>( writeHandle ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_FileWriteStreamOpen( const char * pchFile )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_FileWriteStreamOpen( pchFile ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamRemoteStorage_GetCachedUGCCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_RemoteStorage_GetCachedUGCCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_GetCachedUGCHandle( int32_t iCachedContent )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_GetCachedUGCHandle( static_cast<int32>( iCachedContent ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamRemoteStorage_GetFileCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_RemoteStorage_GetFileCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamRemoteStorage_GetFileSize( const char * pchFile )
{
	try
	{
		return static_cast<int32_t>( Steam_RemoteStorage_GetFileSize( pchFile ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int64_t SWS_SteamAPI_ISteamRemoteStorage_GetFileTimestamp( const char * pchFile )
{
	try
	{
		return static_cast<int64_t>( Steam_RemoteStorage_GetFileTimestamp( pchFile ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamRemoteStorage_GetLocalFileChangeCount( void )
{
	try
	{
		return static_cast<int32_t>( Steam_RemoteStorage_GetLocalFileChangeCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_GetPublishedFileDetails( uint64_t unPublishedFileId, uint32_t unMaxSecondsOld )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_GetPublishedFileDetails( static_cast<PublishedFileId_t>( unPublishedFileId ), static_cast<uint32>( unMaxSecondsOld ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_GetPublishedItemVoteDetails( uint64_t unPublishedFileId )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_GetPublishedItemVoteDetails( static_cast<PublishedFileId_t>( unPublishedFileId ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamRemoteStorage_GetSyncPlatforms( const char * pchFile )
{
	try
	{
		return static_cast<int32_t>( Steam_RemoteStorage_GetSyncPlatforms( pchFile ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_GetUserPublishedItemVoteDetails( uint64_t unPublishedFileId )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_GetUserPublishedItemVoteDetails( static_cast<PublishedFileId_t>( unPublishedFileId ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_IsCloudEnabledForAccount( void )
{
	try
	{
		return Steam_RemoteStorage_IsCloudEnabledForAccount();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_IsCloudEnabledForApp( void )
{
	try
	{
		return Steam_RemoteStorage_IsCloudEnabledForApp();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamRemoteStorage_SetCloudEnabledForApp( bool bEnabled )
{
	try
	{
		Steam_RemoteStorage_SetCloudEnabledForApp( bEnabled );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_SetSyncPlatforms( const char * pchFile, int32_t eRemoteStoragePlatform )
{
	try
	{
		return Steam_RemoteStorage_SetSyncPlatforms( pchFile, static_cast<ERemoteStoragePlatform>( eRemoteStoragePlatform ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_SetUserPublishedFileAction( uint64_t unPublishedFileId, int32_t eAction )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_SetUserPublishedFileAction( static_cast<PublishedFileId_t>( unPublishedFileId ), static_cast<EWorkshopFileAction>( eAction ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_SubscribePublishedFile( uint64_t unPublishedFileId )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_SubscribePublishedFile( static_cast<PublishedFileId_t>( unPublishedFileId ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_UGCDownload( uint64_t hContent, uint32_t unPriority )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_UGCDownload( static_cast<UGCHandle_t>( hContent ), static_cast<uint32>( unPriority ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_UGCDownloadToLocation( uint64_t hContent, const char * pchLocation, uint32_t unPriority )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_UGCDownloadToLocation( static_cast<UGCHandle_t>( hContent ), pchLocation, static_cast<uint32>( unPriority ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_UnsubscribePublishedFile( uint64_t unPublishedFileId )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_UnsubscribePublishedFile( static_cast<PublishedFileId_t>( unPublishedFileId ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_UpdatePublishedFileDescription( uint64_t updateHandle, const char * pchDescription )
{
	try
	{
		return Steam_RemoteStorage_UpdatePublishedFileDescription( static_cast<PublishedFileUpdateHandle_t>( updateHandle ), pchDescription );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_UpdatePublishedFileFile( uint64_t updateHandle, const char * pchFile )
{
	try
	{
		return Steam_RemoteStorage_UpdatePublishedFileFile( static_cast<PublishedFileUpdateHandle_t>( updateHandle ), pchFile );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_UpdatePublishedFilePreviewFile( uint64_t updateHandle, const char * pchPreviewFile )
{
	try
	{
		return Steam_RemoteStorage_UpdatePublishedFilePreviewFile( static_cast<PublishedFileUpdateHandle_t>( updateHandle ), pchPreviewFile );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription( uint64_t updateHandle, const char * pchChangeDescription )
{
	try
	{
		return Steam_RemoteStorage_UpdatePublishedFileSetChangeDescription( static_cast<PublishedFileUpdateHandle_t>( updateHandle ), pchChangeDescription );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTitle( uint64_t updateHandle, const char * pchTitle )
{
	try
	{
		return Steam_RemoteStorage_UpdatePublishedFileTitle( static_cast<PublishedFileUpdateHandle_t>( updateHandle ), pchTitle );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamRemoteStorage_UpdatePublishedFileVisibility( uint64_t updateHandle, int32_t eVisibility )
{
	try
	{
		return Steam_RemoteStorage_UpdatePublishedFileVisibility( static_cast<PublishedFileUpdateHandle_t>( updateHandle ), static_cast<ERemoteStoragePublishedFileVisibility>( eVisibility ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamRemoteStorage_UpdateUserPublishedItemVote( uint64_t unPublishedFileId, bool bVoteUp )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_UpdateUserPublishedItemVote( static_cast<PublishedFileId_t>( unPublishedFileId ), bVoteUp ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamScreenshots_AddScreenshotToLibrary( const char * pchFilename, const char * pchThumbnailFilename, int32_t nWidth, int32_t nHeight )
{
	try
	{
		return static_cast<uint32_t>( Steam_Screenshots_AddScreenshotToLibrary( pchFilename, pchThumbnailFilename, static_cast<int>( nWidth ), static_cast<int>( nHeight ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamScreenshots_AddVRScreenshotToLibrary( int32_t eType, const char * pchFilename, const char * pchVRFilename )
{
	try
	{
		return static_cast<uint32_t>( Steam_Screenshots_AddVRScreenshotToLibrary( static_cast<EVRScreenshotType>( eType ), pchFilename, pchVRFilename ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamScreenshots_HookScreenshots( bool bHook )
{
	try
	{
		Steam_Screenshots_HookScreenshots( bHook );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamScreenshots_IsScreenshotsHooked( void )
{
	try
	{
		return Steam_Screenshots_IsScreenshotsHooked();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamScreenshots_SetLocation( uint32_t hScreenshot, const char * pchLocation )
{
	try
	{
		return Steam_Screenshots_SetLocation( static_cast<ScreenshotHandle>( hScreenshot ), pchLocation );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamScreenshots_TagPublishedFile( uint32_t hScreenshot, uint64_t unPublishedFileID )
{
	try
	{
		return Steam_Screenshots_TagPublishedFile( static_cast<ScreenshotHandle>( hScreenshot ), static_cast<PublishedFileId_t>( unPublishedFileID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamScreenshots_TagUser( uint32_t hScreenshot, uint64_t steamID )
{
	try
	{
		return Steam_Screenshots_TagUser( static_cast<ScreenshotHandle>( hScreenshot ), static_cast<uint64_steamid>( steamID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamScreenshots_TriggerScreenshot( void )
{
	try
	{
		Steam_Screenshots_TriggerScreenshot();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_AddGamePhaseTag( const char * pchTagName, const char * pchTagIcon, const char * pchTagGroup, uint32_t unPriority )
{
	try
	{
		Steam_Timeline_AddGamePhaseTag( pchTagName, pchTagIcon, pchTagGroup, static_cast<uint32>( unPriority ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamTimeline_AddInstantaneousTimelineEvent( const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32_t unIconPriority, float flStartOffsetSeconds, int32_t ePossibleClip )
{
	try
	{
		return static_cast<uint64_t>( Steam_Timeline_AddInstantaneousTimelineEvent( pchTitle, pchDescription, pchIcon, static_cast<uint32>( unIconPriority ), flStartOffsetSeconds, static_cast<ETimelineEventClipPriority>( ePossibleClip ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamTimeline_AddRangeTimelineEvent( const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32_t unIconPriority, float flStartOffsetSeconds, float flDuration, int32_t ePossibleClip )
{
	try
	{
		return static_cast<uint64_t>( Steam_Timeline_AddRangeTimelineEvent( pchTitle, pchDescription, pchIcon, static_cast<uint32>( unIconPriority ), flStartOffsetSeconds, flDuration, static_cast<ETimelineEventClipPriority>( ePossibleClip ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamTimeline_ClearTimelineTooltip( float flTimeDelta )
{
	try
	{
		Steam_Timeline_ClearTimelineTooltip( flTimeDelta );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamTimeline_DoesEventRecordingExist( uint64_t ulEvent )
{
	try
	{
		return static_cast<uint64_t>( Steam_Timeline_DoesEventRecordingExist( static_cast<TimelineEventHandle_t>( ulEvent ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamTimeline_DoesGamePhaseRecordingExist( const char * pchPhaseID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Timeline_DoesGamePhaseRecordingExist( pchPhaseID ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamTimeline_EndGamePhase( void )
{
	try
	{
		Steam_Timeline_EndGamePhase();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_EndRangeTimelineEvent( uint64_t ulEvent, float flEndOffsetSeconds )
{
	try
	{
		Steam_Timeline_EndRangeTimelineEvent( static_cast<TimelineEventHandle_t>( ulEvent ), flEndOffsetSeconds );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_OpenOverlayToGamePhase( const char * pchPhaseID )
{
	try
	{
		Steam_Timeline_OpenOverlayToGamePhase( pchPhaseID );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_OpenOverlayToTimelineEvent( uint64_t ulEvent )
{
	try
	{
		Steam_Timeline_OpenOverlayToTimelineEvent( static_cast<const TimelineEventHandle_t>( ulEvent ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_RemoveTimelineEvent( uint64_t ulEvent )
{
	try
	{
		Steam_Timeline_RemoveTimelineEvent( static_cast<TimelineEventHandle_t>( ulEvent ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_SetGamePhaseAttribute( const char * pchAttributeGroup, const char * pchAttributeValue, uint32_t unPriority )
{
	try
	{
		Steam_Timeline_SetGamePhaseAttribute( pchAttributeGroup, pchAttributeValue, static_cast<uint32>( unPriority ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_SetGamePhaseID( const char * pchPhaseID )
{
	try
	{
		Steam_Timeline_SetGamePhaseID( pchPhaseID );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_SetTimelineGameMode( int32_t eMode )
{
	try
	{
		Steam_Timeline_SetTimelineGameMode( static_cast<ETimelineGameMode>( eMode ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_SetTimelineTooltip( const char * pchDescription, float flTimeDelta )
{
	try
	{
		Steam_Timeline_SetTimelineTooltip( pchDescription, flTimeDelta );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamTimeline_StartGamePhase( void )
{
	try
	{
		Steam_Timeline_StartGamePhase();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamTimeline_StartRangeTimelineEvent( const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32_t unPriority, float flStartOffsetSeconds, int32_t ePossibleClip )
{
	try
	{
		return static_cast<uint64_t>( Steam_Timeline_StartRangeTimelineEvent( pchTitle, pchDescription, pchIcon, static_cast<uint32>( unPriority ), flStartOffsetSeconds, static_cast<ETimelineEventClipPriority>( ePossibleClip ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamTimeline_UpdateRangeTimelineEvent( uint64_t ulEvent, const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32_t unPriority, int32_t ePossibleClip )
{
	try
	{
		Steam_Timeline_UpdateRangeTimelineEvent( static_cast<TimelineEventHandle_t>( ulEvent ), pchTitle, pchDescription, pchIcon, static_cast<uint32>( unPriority ), static_cast<ETimelineEventClipPriority>( ePossibleClip ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_AddAppDependency( uint64_t nPublishedFileID, uint32_t nAppID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_AddAppDependency( static_cast<PublishedFileId_t>( nPublishedFileID ), static_cast<AppId_t>( nAppID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_AddContentDescriptor( uint64_t handle, int32_t descid )
{
	try
	{
		return Steam_UGC_AddContentDescriptor( static_cast<UGCUpdateHandle_t>( handle ), static_cast<EUGCContentDescriptorID>( descid ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_AddDependency( uint64_t nParentPublishedFileID, uint64_t nChildPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_AddDependency( static_cast<PublishedFileId_t>( nParentPublishedFileID ), static_cast<PublishedFileId_t>( nChildPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_AddExcludedTag( uint64_t handle, const char * pTagName )
{
	try
	{
		return Steam_UGC_AddExcludedTag( static_cast<UGCQueryHandle_t>( handle ), pTagName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_AddItemKeyValueTag( uint64_t handle, const char * pchKey, const char * pchValue )
{
	try
	{
		return Steam_UGC_AddItemKeyValueTag( static_cast<UGCUpdateHandle_t>( handle ), pchKey, pchValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_AddItemPreviewFile( uint64_t handle, const char * pszPreviewFile, int32_t type )
{
	try
	{
		return Steam_UGC_AddItemPreviewFile( static_cast<UGCUpdateHandle_t>( handle ), pszPreviewFile, static_cast<EItemPreviewType>( type ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_AddItemPreviewVideo( uint64_t handle, const char * pszVideoID )
{
	try
	{
		return Steam_UGC_AddItemPreviewVideo( static_cast<UGCUpdateHandle_t>( handle ), pszVideoID );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_AddItemToFavorites( uint32_t nAppId, uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_AddItemToFavorites( static_cast<AppId_t>( nAppId ), static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_AddRequiredKeyValueTag( uint64_t handle, const char * pKey, const char * pValue )
{
	try
	{
		return Steam_UGC_AddRequiredKeyValueTag( static_cast<UGCQueryHandle_t>( handle ), pKey, pValue );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_AddRequiredTag( uint64_t handle, const char * pTagName )
{
	try
	{
		return Steam_UGC_AddRequiredTag( static_cast<UGCQueryHandle_t>( handle ), pTagName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_BInitWorkshopForGameServer( uint32_t unWorkshopDepotID, const char * pszFolder )
{
	try
	{
		return Steam_UGC_BInitWorkshopForGameServer( static_cast<DepotId_t>( unWorkshopDepotID ), pszFolder );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_CreateItem( uint32_t nConsumerAppId, int32_t eFileType )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_CreateItem( static_cast<AppId_t>( nConsumerAppId ), static_cast<EWorkshopFileType>( eFileType ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_CreateQueryAllUGCRequestCursor( int32_t eQueryType, int32_t eMatchingeMatchingUGCTypeFileType, uint32_t nCreatorAppID, uint32_t nConsumerAppID, const char * pchCursor )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_CreateQueryAllUGCRequest( static_cast<EUGCQuery>( eQueryType ), static_cast<EUGCMatchingUGCType>( eMatchingeMatchingUGCTypeFileType ), static_cast<AppId_t>( nCreatorAppID ), static_cast<AppId_t>( nConsumerAppID ), pchCursor ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_CreateQueryAllUGCRequestPage( int32_t eQueryType, int32_t eMatchingeMatchingUGCTypeFileType, uint32_t nCreatorAppID, uint32_t nConsumerAppID, uint32_t unPage )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_CreateQueryAllUGCRequest( static_cast<EUGCQuery>( eQueryType ), static_cast<EUGCMatchingUGCType>( eMatchingeMatchingUGCTypeFileType ), static_cast<AppId_t>( nCreatorAppID ), static_cast<AppId_t>( nConsumerAppID ), static_cast<uint32>( unPage ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_CreateQueryUserUGCRequest( uint32_t unAccountID, int32_t eListType, int32_t eMatchingUGCType, int32_t eSortOrder, uint32_t nCreatorAppID, uint32_t nConsumerAppID, uint32_t unPage )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_CreateQueryUserUGCRequest( static_cast<AccountID_t>( unAccountID ), static_cast<EUserUGCList>( eListType ), static_cast<EUGCMatchingUGCType>( eMatchingUGCType ), static_cast<EUserUGCListSortOrder>( eSortOrder ), static_cast<AppId_t>( nCreatorAppID ), static_cast<AppId_t>( nConsumerAppID ), static_cast<uint32>( unPage ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_DeleteItem( uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_DeleteItem( static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_DownloadItem( uint64_t nPublishedFileID, bool bHighPriority )
{
	try
	{
		return Steam_UGC_DownloadItem( static_cast<PublishedFileId_t>( nPublishedFileID ), bHighPriority );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_GetAppDependencies( uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_GetAppDependencies( static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUGC_GetItemState( uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint32_t>( Steam_UGC_GetItemState( static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUGC_GetNumDownloadedItems( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_UGC_GetNumDownloadedItems() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUGC_GetNumSubscribedItems( bool bIncludeLocallyDisabled )
{
	try
	{
		return static_cast<uint32_t>( Steam_UGC_GetNumSubscribedItems( bIncludeLocallyDisabled ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUGC_GetNumSupportedGameVersions( uint64_t handle, uint32_t index )
{
	try
	{
		return static_cast<uint32_t>( Steam_UGC_GetNumSupportedGameVersions( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUGC_GetQueryUGCNumAdditionalPreviews( uint64_t handle, uint32_t index )
{
	try
	{
		return static_cast<uint32_t>( Steam_UGC_GetQueryUGCNumAdditionalPreviews( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUGC_GetQueryUGCNumKeyValueTags( uint64_t handle, uint32_t index )
{
	try
	{
		return static_cast<uint32_t>( Steam_UGC_GetQueryUGCNumKeyValueTags( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUGC_GetQueryUGCNumTags( uint64_t handle, uint32_t index )
{
	try
	{
		return static_cast<uint32_t>( Steam_UGC_GetQueryUGCNumTags( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_GetUserItemVote( uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_GetUserItemVote( static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_GetWorkshopEULAStatus( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_GetWorkshopEULAStatus() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_MarkDownloadedItemAsUnused( uint64_t nPublishedFileID )
{
	try
	{
		return Steam_UGC_MarkDownloadedItemAsUnused( static_cast<PublishedFileId_t>( nPublishedFileID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_ReleaseQueryUGCRequest( uint64_t handle )
{
	try
	{
		return Steam_UGC_ReleaseQueryUGCRequest( static_cast<UGCQueryHandle_t>( handle ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_RemoveAllItemKeyValueTags( uint64_t handle )
{
	try
	{
		return Steam_UGC_RemoveAllItemKeyValueTags( static_cast<UGCUpdateHandle_t>( handle ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_RemoveAppDependency( uint64_t nPublishedFileID, uint32_t nAppID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_RemoveAppDependency( static_cast<PublishedFileId_t>( nPublishedFileID ), static_cast<AppId_t>( nAppID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_RemoveContentDescriptor( uint64_t handle, int32_t descid )
{
	try
	{
		return Steam_UGC_RemoveContentDescriptor( static_cast<UGCUpdateHandle_t>( handle ), static_cast<EUGCContentDescriptorID>( descid ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_RemoveDependency( uint64_t nParentPublishedFileID, uint64_t nChildPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_RemoveDependency( static_cast<PublishedFileId_t>( nParentPublishedFileID ), static_cast<PublishedFileId_t>( nChildPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_RemoveItemFromFavorites( uint32_t nAppId, uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_RemoveItemFromFavorites( static_cast<AppId_t>( nAppId ), static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_RemoveItemKeyValueTags( uint64_t handle, const char * pchKey )
{
	try
	{
		return Steam_UGC_RemoveItemKeyValueTags( static_cast<UGCUpdateHandle_t>( handle ), pchKey );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_RemoveItemPreview( uint64_t handle, uint32_t index )
{
	try
	{
		return Steam_UGC_RemoveItemPreview( static_cast<UGCUpdateHandle_t>( handle ), static_cast<uint32>( index ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_RequestUGCDetails( uint64_t nPublishedFileID, uint32_t unMaxAgeSeconds )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_RequestUGCDetails( static_cast<PublishedFileId_t>( nPublishedFileID ), static_cast<uint32>( unMaxAgeSeconds ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_SendQueryUGCRequest( uint64_t handle )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_SendQueryUGCRequest( static_cast<UGCQueryHandle_t>( handle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_SetAdminQuery( uint64_t handle, bool bAdminQuery )
{
	try
	{
		return Steam_UGC_SetAdminQuery( static_cast<UGCUpdateHandle_t>( handle ), bAdminQuery );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetAllowCachedResponse( uint64_t handle, uint32_t unMaxAgeSeconds )
{
	try
	{
		return Steam_UGC_SetAllowCachedResponse( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( unMaxAgeSeconds ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetAllowLegacyUpload( uint64_t handle, bool bAllowLegacyUpload )
{
	try
	{
		return Steam_UGC_SetAllowLegacyUpload( static_cast<UGCUpdateHandle_t>( handle ), bAllowLegacyUpload );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetCloudFileNameFilter( uint64_t handle, const char * pMatchCloudFileName )
{
	try
	{
		return Steam_UGC_SetCloudFileNameFilter( static_cast<UGCQueryHandle_t>( handle ), pMatchCloudFileName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetItemContent( uint64_t handle, const char * pszContentFolder )
{
	try
	{
		return Steam_UGC_SetItemContent( static_cast<UGCUpdateHandle_t>( handle ), pszContentFolder );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetItemDescription( uint64_t handle, const char * pchDescription )
{
	try
	{
		return Steam_UGC_SetItemDescription( static_cast<UGCUpdateHandle_t>( handle ), pchDescription );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetItemMetadata( uint64_t handle, const char * pchMetaData )
{
	try
	{
		return Steam_UGC_SetItemMetadata( static_cast<UGCUpdateHandle_t>( handle ), pchMetaData );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetItemPreview( uint64_t handle, const char * pszPreviewFile )
{
	try
	{
		return Steam_UGC_SetItemPreview( static_cast<UGCUpdateHandle_t>( handle ), pszPreviewFile );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetItemTitle( uint64_t handle, const char * pchTitle )
{
	try
	{
		return Steam_UGC_SetItemTitle( static_cast<UGCUpdateHandle_t>( handle ), pchTitle );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetItemUpdateLanguage( uint64_t handle, const char * pchLanguage )
{
	try
	{
		return Steam_UGC_SetItemUpdateLanguage( static_cast<UGCUpdateHandle_t>( handle ), pchLanguage );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetItemVisibility( uint64_t handle, int32_t eVisibility )
{
	try
	{
		return Steam_UGC_SetItemVisibility( static_cast<UGCUpdateHandle_t>( handle ), static_cast<ERemoteStoragePublishedFileVisibility>( eVisibility ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetLanguage( uint64_t handle, const char * pchLanguage )
{
	try
	{
		return Steam_UGC_SetLanguage( static_cast<UGCQueryHandle_t>( handle ), pchLanguage );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetMatchAnyTag( uint64_t handle, bool bMatchAnyTag )
{
	try
	{
		return Steam_UGC_SetMatchAnyTag( static_cast<UGCQueryHandle_t>( handle ), bMatchAnyTag );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetRankedByTrendDays( uint64_t handle, uint32_t unDays )
{
	try
	{
		return Steam_UGC_SetRankedByTrendDays( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( unDays ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetRequiredGameVersions( uint64_t handle, const char * pszGameBranchMin, const char * pszGameBranchMax )
{
	try
	{
		return Steam_UGC_SetRequiredGameVersions( static_cast<UGCUpdateHandle_t>( handle ), pszGameBranchMin, pszGameBranchMax );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetReturnAdditionalPreviews( uint64_t handle, bool bReturnAdditionalPreviews )
{
	try
	{
		return Steam_UGC_SetReturnAdditionalPreviews( static_cast<UGCQueryHandle_t>( handle ), bReturnAdditionalPreviews );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetReturnChildren( uint64_t handle, bool bReturnChildren )
{
	try
	{
		return Steam_UGC_SetReturnChildren( static_cast<UGCQueryHandle_t>( handle ), bReturnChildren );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetReturnKeyValueTags( uint64_t handle, bool bReturnKeyValueTags )
{
	try
	{
		return Steam_UGC_SetReturnKeyValueTags( static_cast<UGCQueryHandle_t>( handle ), bReturnKeyValueTags );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetReturnLongDescription( uint64_t handle, bool bReturnLongDescription )
{
	try
	{
		return Steam_UGC_SetReturnLongDescription( static_cast<UGCQueryHandle_t>( handle ), bReturnLongDescription );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetReturnMetadata( uint64_t handle, bool bReturnMetadata )
{
	try
	{
		return Steam_UGC_SetReturnMetadata( static_cast<UGCQueryHandle_t>( handle ), bReturnMetadata );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetReturnOnlyIDs( uint64_t handle, bool bReturnOnlyIDs )
{
	try
	{
		return Steam_UGC_SetReturnOnlyIDs( static_cast<UGCQueryHandle_t>( handle ), bReturnOnlyIDs );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetReturnPlaytimeStats( uint64_t handle, uint32_t unDays )
{
	try
	{
		return Steam_UGC_SetReturnPlaytimeStats( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( unDays ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetReturnTotalOnly( uint64_t handle, bool bReturnTotalOnly )
{
	try
	{
		return Steam_UGC_SetReturnTotalOnly( static_cast<UGCQueryHandle_t>( handle ), bReturnTotalOnly );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetSearchText( uint64_t handle, const char * pSearchText )
{
	try
	{
		return Steam_UGC_SetSearchText( static_cast<UGCQueryHandle_t>( handle ), pSearchText );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetTimeCreatedDateRange( uint64_t handle, uint32_t rtStart, uint32_t rtEnd )
{
	try
	{
		return Steam_UGC_SetTimeCreatedDateRange( static_cast<UGCQueryHandle_t>( handle ), static_cast<RTime32>( rtStart ), static_cast<RTime32>( rtEnd ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_SetTimeUpdatedDateRange( uint64_t handle, uint32_t rtStart, uint32_t rtEnd )
{
	try
	{
		return Steam_UGC_SetTimeUpdatedDateRange( static_cast<UGCQueryHandle_t>( handle ), static_cast<RTime32>( rtStart ), static_cast<RTime32>( rtEnd ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_SetUserItemVote( uint64_t nPublishedFileID, bool bVoteUp )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_SetUserItemVote( static_cast<PublishedFileId_t>( nPublishedFileID ), bVoteUp ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_ShowWorkshopEULA( void )
{
	try
	{
		return Steam_UGC_ShowWorkshopEULA();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_StartItemUpdate( uint32_t nConsumerAppId, uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_StartItemUpdate( static_cast<AppId_t>( nConsumerAppId ), static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_StopPlaytimeTrackingForAllItems( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_StopPlaytimeTrackingForAllItems() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_SubmitItemUpdate( uint64_t handle, const char * pchChangeNote )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_SubmitItemUpdate( static_cast<UGCUpdateHandle_t>( handle ), pchChangeNote ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_SubscribeItem( uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_SubscribeItem( static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamUGC_SuspendDownloads( bool bSuspend )
{
	try
	{
		Steam_UGC_SuspendDownloads( bSuspend );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_SteamAPI_ISteamUGC_UnsubscribeItem( uint64_t nPublishedFileID )
{
	try
	{
		return static_cast<uint64_t>( Steam_UGC_UnsubscribeItem( static_cast<PublishedFileId_t>( nPublishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUGC_UpdateItemPreviewFile( uint64_t handle, uint32_t index, const char * pszPreviewFile )
{
	try
	{
		return Steam_UGC_UpdateItemPreviewFile( static_cast<UGCUpdateHandle_t>( handle ), static_cast<uint32>( index ), pszPreviewFile );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUGC_UpdateItemPreviewVideo( uint64_t handle, uint32_t index, const char * pszVideoID )
{
	try
	{
		return Steam_UGC_UpdateItemPreviewVideo( static_cast<UGCUpdateHandle_t>( handle ), static_cast<uint32>( index ), pszVideoID );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUserStats_AttachLeaderboardUGC( uint64_t hSteamLeaderboard, uint64_t hUGC )
{
	try
	{
		return static_cast<uint64_t>( Steam_UserStats_AttachLeaderboardUGC( static_cast<SteamLeaderboard_t>( hSteamLeaderboard ), static_cast<UGCHandle_t>( hUGC ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUserStats_ClearAchievement( const char * pchName )
{
	try
	{
		return Steam_UserStats_ClearAchievement( pchName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUserStats_DownloadLeaderboardEntries( uint64_t hSteamLeaderboard, int32_t eLeaderboardDataRequest, int32_t nRangeStart, int32_t nRangeEnd )
{
	try
	{
		return static_cast<uint64_t>( Steam_UserStats_DownloadLeaderboardEntries( static_cast<SteamLeaderboard_t>( hSteamLeaderboard ), static_cast<ELeaderboardDataRequest>( eLeaderboardDataRequest ), static_cast<int>( nRangeStart ), static_cast<int>( nRangeEnd ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUserStats_FindLeaderboard( const char * pchLeaderboardName )
{
	try
	{
		return static_cast<uint64_t>( Steam_UserStats_FindLeaderboard( pchLeaderboardName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUserStats_FindOrCreateLeaderboard( const char * pchLeaderboardName, int32_t eLeaderboardSortMethod, int32_t eLeaderboardDisplayType )
{
	try
	{
		return static_cast<uint64_t>( Steam_UserStats_FindOrCreateLeaderboard( pchLeaderboardName, static_cast<ELeaderboardSortMethod>( eLeaderboardSortMethod ), static_cast<ELeaderboardDisplayType>( eLeaderboardDisplayType ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute( const char * pchName, const char * pchKey )
{
	try
	{
		return Steam_UserStats_GetAchievementDisplayAttribute( pchName, pchKey );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamUserStats_GetAchievementIcon( const char * pchName )
{
	try
	{
		return static_cast<int32_t>( Steam_UserStats_GetAchievementIcon( pchName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamUserStats_GetAchievementName( uint32_t iAchievement )
{
	try
	{
		return Steam_UserStats_GetAchievementName( static_cast<uint32>( iAchievement ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamUserStats_GetLeaderboardDisplayType( uint64_t hSteamLeaderboard )
{
	try
	{
		return static_cast<int32_t>( Steam_UserStats_GetLeaderboardDisplayType( static_cast<SteamLeaderboard_t>( hSteamLeaderboard ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamUserStats_GetLeaderboardEntryCount( uint64_t hSteamLeaderboard )
{
	try
	{
		return static_cast<int32_t>( Steam_UserStats_GetLeaderboardEntryCount( static_cast<SteamLeaderboard_t>( hSteamLeaderboard ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamUserStats_GetLeaderboardName( uint64_t hSteamLeaderboard )
{
	try
	{
		return Steam_UserStats_GetLeaderboardName( static_cast<SteamLeaderboard_t>( hSteamLeaderboard ) );
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamUserStats_GetLeaderboardSortMethod( uint64_t hSteamLeaderboard )
{
	try
	{
		return static_cast<int32_t>( Steam_UserStats_GetLeaderboardSortMethod( static_cast<SteamLeaderboard_t>( hSteamLeaderboard ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUserStats_GetNumAchievements( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_UserStats_GetNumAchievements() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUserStats_GetNumberOfCurrentPlayers( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_UserStats_GetNumberOfCurrentPlayers() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUserStats_IndicateAchievementProgress( const char * pchName, uint32_t nCurProgress, uint32_t nMaxProgress )
{
	try
	{
		return Steam_UserStats_IndicateAchievementProgress( pchName, static_cast<uint32>( nCurProgress ), static_cast<uint32>( nMaxProgress ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUserStats_RequestGlobalAchievementPercentages( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_UserStats_RequestGlobalAchievementPercentages() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUserStats_RequestGlobalStats( int32_t nHistoryDays )
{
	try
	{
		return static_cast<uint64_t>( Steam_UserStats_RequestGlobalStats( static_cast<int>( nHistoryDays ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUserStats_RequestUserStats( uint64_t steamIDUser )
{
	try
	{
		return static_cast<uint64_t>( Steam_UserStats_RequestUserStats( static_cast<uint64_steamid>( steamIDUser ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUserStats_ResetAllStats( bool bAchievementsToo )
{
	try
	{
		return Steam_UserStats_ResetAllStats( bAchievementsToo );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUserStats_SetAchievement( const char * pchName )
{
	try
	{
		return Steam_UserStats_SetAchievement( pchName );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUserStats_SetStatFloat( const char * pchName, float fData )
{
	try
	{
		return Steam_UserStats_SetStat( pchName, fData );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUserStats_SetStatInt32( const char * pchName, int32_t nData )
{
	try
	{
		return Steam_UserStats_SetStat( pchName, static_cast<int32>( nData ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUserStats_StoreStats( void )
{
	try
	{
		return Steam_UserStats_StoreStats();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUserStats_UpdateAvgRateStat( const char * pchName, float flCountThisSession, double dSessionLength )
{
	try
	{
		return Steam_UserStats_UpdateAvgRateStat( pchName, flCountThisSession, dSessionLength );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamUser_AdvertiseGame( uint64_t steamIDGameServer, uint32_t unIPServer, uint16_t usPortServer )
{
	try
	{
		Steam_User_AdvertiseGame( static_cast<uint64_steamid>( steamIDGameServer ), static_cast<uint32>( unIPServer ), static_cast<uint16>( usPortServer ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamUser_BIsBehindNAT( void )
{
	try
	{
		return Steam_User_BIsBehindNAT();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUser_BIsPhoneIdentifying( void )
{
	try
	{
		return Steam_User_BIsPhoneIdentifying();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUser_BIsPhoneRequiringVerification( void )
{
	try
	{
		return Steam_User_BIsPhoneRequiringVerification();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUser_BIsPhoneVerified( void )
{
	try
	{
		return Steam_User_BIsPhoneVerified();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUser_BIsTwoFactorEnabled( void )
{
	try
	{
		return Steam_User_BIsTwoFactorEnabled();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUser_BLoggedOn( void )
{
	try
	{
		return Steam_User_BLoggedOn();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUser_BSetDurationControlOnlineState( int32_t eNewState )
{
	try
	{
		return Steam_User_BSetDurationControlOnlineState( static_cast<EDurationControlOnlineState>( eNewState ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamUser_CancelAuthTicket( uint32_t hAuthTicket )
{
	try
	{
		Steam_User_CancelAuthTicket( static_cast<HAuthTicket>( hAuthTicket ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamUser_EndAuthSession( uint64_t steamID )
{
	try
	{
		Steam_User_EndAuthSession( static_cast<uint64_steamid>( steamID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint32_t SWS_SteamAPI_ISteamUser_GetAuthTicketForWebApi( const char * pchIdentity )
{
	try
	{
		return static_cast<uint32_t>( Steam_User_GetAuthTicketForWebApi( pchIdentity ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUser_GetDurationControl( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_User_GetDurationControl() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamUser_GetGameBadgeLevel( int32_t nSeries, bool bFoil )
{
	try
	{
		return static_cast<int32_t>( Steam_User_GetGameBadgeLevel( static_cast<int>( nSeries ), bFoil ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamUser_GetHSteamUser( void )
{
	try
	{
		return static_cast<int32_t>( Steam_User_GetHSteamUser() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUser_GetMarketEligibility( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_User_GetMarketEligibility() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamUser_GetPlayerSteamLevel( void )
{
	try
	{
		return static_cast<int32_t>( Steam_User_GetPlayerSteamLevel() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUser_GetSteamID( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_User_GetSteamID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUser_GetVoiceOptimalSampleRate( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_User_GetVoiceOptimalSampleRate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_SteamAPI_ISteamUser_RequestStoreAuthURL( const char * pchRedirectURL )
{
	try
	{
		return static_cast<uint64_t>( Steam_User_RequestStoreAuthURL( pchRedirectURL ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_SteamAPI_ISteamUser_StartVoiceRecording( void )
{
	try
	{
		Steam_User_StartVoiceRecording();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamUser_StopVoiceRecording( void )
{
	try
	{
		Steam_User_StopVoiceRecording();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamUser_TerminateGameConnection_DEPRECATED( uint32_t unIPServer, uint16_t usPortServer )
{
	try
	{
		Steam_User_TerminateGameConnection_DEPRECATED( static_cast<uint32>( unIPServer ), static_cast<uint16>( usPortServer ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamUser_TrackAppUsageEvent( uint64_t gameID, int32_t eAppUsageEvent, const char * pchExtraInfo )
{
	try
	{
		Steam_User_TrackAppUsageEvent( static_cast<uint64_gameid>( gameID ), static_cast<int>( eAppUsageEvent ), pchExtraInfo );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_SteamAPI_ISteamUser_UserHasLicenseForApp( uint64_t steamID, uint32_t appID )
{
	try
	{
		return static_cast<int32_t>( Steam_User_UserHasLicenseForApp( static_cast<uint64_steamid>( steamID ), static_cast<AppId_t>( appID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUtils_BOverlayNeedsPresent( void )
{
	try
	{
		return Steam_Utils_BOverlayNeedsPresent();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_SteamAPI_ISteamUtils_CheckFileSignature( const char * szFileName )
{
	try
	{
		return static_cast<uint64_t>( Steam_Utils_CheckFileSignature( szFileName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUtils_DismissFloatingGamepadTextInput( void )
{
	try
	{
		return Steam_Utils_DismissFloatingGamepadTextInput();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUtils_DismissGamepadTextInput( void )
{
	try
	{
		return Steam_Utils_DismissGamepadTextInput();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamUtils_GetAPICallFailureReason( uint64_t hSteamAPICall )
{
	try
	{
		return static_cast<int32_t>( Steam_Utils_GetAPICallFailureReason( static_cast<SteamAPICall_t>( hSteamAPICall ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUtils_GetAppID( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Utils_GetAppID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamUtils_GetConnectedUniverse( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Utils_GetConnectedUniverse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint8_t SWS_SteamAPI_ISteamUtils_GetCurrentBatteryPower( void )
{
	try
	{
		return static_cast<uint8_t>( Steam_Utils_GetCurrentBatteryPower() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUtils_GetEnteredGamepadTextLength( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Utils_GetEnteredGamepadTextLength() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUtils_GetIPCCallCount( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Utils_GetIPCCallCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamUtils_GetIPCountry( void )
{
	try
	{
		return Steam_Utils_GetIPCountry();
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_SteamAPI_ISteamUtils_GetIPv6ConnectivityState( int32_t eProtocol )
{
	try
	{
		return static_cast<int32_t>( Steam_Utils_GetIPv6ConnectivityState( static_cast<ESteamIPv6ConnectivityProtocol>( eProtocol ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUtils_GetSecondsSinceAppActive( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Utils_GetSecondsSinceAppActive() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUtils_GetSecondsSinceComputerActive( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Utils_GetSecondsSinceComputerActive() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_SteamAPI_ISteamUtils_GetServerRealTime( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Utils_GetServerRealTime() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_SteamAPI_ISteamUtils_GetSteamHardwareDefaultConfig( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Utils_GetSteamHardwareDefaultConfig() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_SteamAPI_ISteamUtils_GetSteamUILanguage( void )
{
	try
	{
		return Steam_Utils_GetSteamUILanguage();
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

bool SWS_SteamAPI_ISteamUtils_InitFilterText( uint32_t unFilterOptions )
{
	try
	{
		return Steam_Utils_InitFilterText( static_cast<uint32>( unFilterOptions ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUtils_IsOverlayEnabled( void )
{
	try
	{
		return Steam_Utils_IsOverlayEnabled();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_SteamAPI_ISteamUtils_IsRunningOnSteamHardware( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Utils_IsRunningOnSteamHardware() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_SteamAPI_ISteamUtils_IsRunningUnderProton( void )
{
	try
	{
		return Steam_Utils_IsRunningUnderProton();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUtils_IsSteamChinaLauncher( void )
{
	try
	{
		return Steam_Utils_IsSteamChinaLauncher();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUtils_IsSteamInBigPictureMode( void )
{
	try
	{
		return Steam_Utils_IsSteamInBigPictureMode();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUtils_IsSteamRunningInVR( void )
{
	try
	{
		return Steam_Utils_IsSteamRunningInVR();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUtils_IsVRHeadsetStreamingEnabled( void )
{
	try
	{
		return Steam_Utils_IsVRHeadsetStreamingEnabled();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamUtils_SetGameLauncherMode( bool bLauncherMode )
{
	try
	{
		Steam_Utils_SetGameLauncherMode( bLauncherMode );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamUtils_SetOverlayNotificationInset( int32_t nHorizontalInset, int32_t nVerticalInset )
{
	try
	{
		Steam_Utils_SetOverlayNotificationInset( static_cast<int>( nHorizontalInset ), static_cast<int>( nVerticalInset ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamUtils_SetOverlayNotificationPosition( int32_t eNotificationPosition )
{
	try
	{
		Steam_Utils_SetOverlayNotificationPosition( static_cast<ENotificationPosition>( eNotificationPosition ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamUtils_SetVRHeadsetStreamingEnabled( bool bEnabled )
{
	try
	{
		Steam_Utils_SetVRHeadsetStreamingEnabled( bEnabled );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput( int32_t eKeyboardMode, int32_t nTextFieldXPosition, int32_t nTextFieldYPosition, int32_t nTextFieldWidth, int32_t nTextFieldHeight )
{
	try
	{
		return Steam_Utils_ShowFloatingGamepadTextInput( static_cast<EFloatingGamepadTextInputMode>( eKeyboardMode ), static_cast<int>( nTextFieldXPosition ), static_cast<int>( nTextFieldYPosition ), static_cast<int>( nTextFieldWidth ), static_cast<int>( nTextFieldHeight ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_SteamAPI_ISteamUtils_ShowGamepadTextInput( int32_t eInputMode, int32_t eLineInputMode, const char * pchDescription, uint32_t unCharMax, const char * pchExistingText )
{
	try
	{
		return Steam_Utils_ShowGamepadTextInput( static_cast<EGamepadTextInputMode>( eInputMode ), static_cast<EGamepadTextInputLineMode>( eLineInputMode ), pchDescription, static_cast<uint32>( unCharMax ), pchExistingText );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_SteamAPI_ISteamUtils_StartVRDashboard( void )
{
	try
	{
		Steam_Utils_StartVRDashboard();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamVideo_GetOPFSettings( uint32_t unVideoAppID )
{
	try
	{
		Steam_Video_GetOPFSettings( static_cast<AppId_t>( unVideoAppID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_SteamAPI_ISteamVideo_GetVideoURL( uint32_t unVideoAppID )
{
	try
	{
		Steam_Video_GetVideoURL( static_cast<AppId_t>( unVideoAppID ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

SWS_String SWS_Steam_Apps_GetAppInstallDirString( uint32_t appID )
{
	try
	{
		return CopyStringForC( Steam_Apps_GetAppInstallDirString( static_cast<AppId_t>( appID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Apps_GetBetaInfoString( int32_t betaIndex )
{
	try
	{
		return CopyStringForC( Steam_Apps_GetBetaInfoString( static_cast<int>( betaIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Apps_GetCurrentBetaNameString( void )
{
	try
	{
		return CopyStringForC( Steam_Apps_GetCurrentBetaNameString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Apps_GetDLCDataByIndexString( int32_t dlcIndex )
{
	try
	{
		return CopyStringForC( Steam_Apps_GetDLCDataByIndexString( static_cast<int>( dlcIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Apps_GetDlcDownloadProgressString( uint32_t appID )
{
	try
	{
		return CopyStringForC( Steam_Apps_GetDlcDownloadProgressString( static_cast<AppId_t>( appID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_Apps_GetInstalledDepotsList( uint32_t appID )
{
	try
	{
		return CopyStringListForC( Steam_Apps_GetInstalledDepotsList( static_cast<AppId_t>( appID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Apps_GetLaunchCommandLineString( void )
{
	try
	{
		return CopyStringForC( Steam_Apps_GetLaunchCommandLineString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Apps_GetNumBetasString( void )
{
	try
	{
		return CopyStringForC( Steam_Apps_GetNumBetasString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Apps_GetTimedTrialString( void )
{
	try
	{
		return CopyStringForC( Steam_Apps_GetTimedTrialString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Apps_IsTimedTrialString( void )
{
	try
	{
		return CopyStringForC( Steam_Apps_IsTimedTrialString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_CallbackDispatchModeAutomatic( void )
{
	try
	{
		return static_cast<int32_t>( Steam_CallbackDispatchModeAutomatic() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_CallbackDispatchModeManual( void )
{
	try
	{
		return static_cast<int32_t>( Steam_CallbackDispatchModeManual() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_CallbackDispatchModeUninitialized( void )
{
	try
	{
		return static_cast<int32_t>( Steam_CallbackDispatchModeUninitialized() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_Steam_ClearHelperState( void )
{
	try
	{
		Steam_ClearHelperState();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

SWS_String SWS_Steam_Controller_GetControllerBindingRevisionString( uint64_t controllerHandle )
{
	try
	{
		return CopyStringForC( Steam_Controller_GetControllerBindingRevisionString( static_cast<ControllerHandle_t>( controllerHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_FriendFlagAll( void )
{
	try
	{
		return static_cast<int32_t>( Steam_FriendFlagAll() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_FriendFlagImmediate( void )
{
	try
	{
		return static_cast<int32_t>( Steam_FriendFlagImmediate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_FriendFlagOnGameServer( void )
{
	try
	{
		return static_cast<int32_t>( Steam_FriendFlagOnGameServer() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Friends_GetClanActivityCountsString( uint64_t steamIDClan )
{
	try
	{
		return CopyStringForC( Steam_Friends_GetClanActivityCountsString( static_cast<uint64_steamid>( steamIDClan ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_Friends_GetFriendByIndexImmediate( int32_t index )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetFriendByIndexImmediate( static_cast<int>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_Friends_GetFriendCountImmediate( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Friends_GetFriendCountImmediate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_Friends_GetFriendGameAppID( uint64_t friendID )
{
	try
	{
		return static_cast<uint32_t>( Steam_Friends_GetFriendGameAppID( static_cast<uint64_steamid>( friendID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_Friends_GetFriendGameID( uint64_t friendID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetFriendGameID( static_cast<uint64_steamid>( friendID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_Friends_GetFriendGameIP( uint64_t friendID )
{
	try
	{
		return static_cast<uint32_t>( Steam_Friends_GetFriendGameIP( static_cast<uint64_steamid>( friendID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_Friends_GetFriendGameLobbyID( uint64_t friendID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Friends_GetFriendGameLobbyID( static_cast<uint64_steamid>( friendID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_Friends_GetFriendGamePlayedInfo( uint64_t friendID )
{
	try
	{
		return Steam_Friends_GetFriendGamePlayedInfo( static_cast<uint64_steamid>( friendID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint16_t SWS_Steam_Friends_GetFriendGamePort( uint64_t friendID )
{
	try
	{
		return static_cast<uint16_t>( Steam_Friends_GetFriendGamePort( static_cast<uint64_steamid>( friendID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint16_t SWS_Steam_Friends_GetFriendGameQueryPort( uint64_t friendID )
{
	try
	{
		return static_cast<uint16_t>( Steam_Friends_GetFriendGameQueryPort( static_cast<uint64_steamid>( friendID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_Friends_IsFriendInCurrentGame( uint64_t friendID )
{
	try
	{
		return Steam_Friends_IsFriendInCurrentGame( static_cast<uint64_steamid>( friendID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_Bytes SWS_Steam_GameServerHTTP_GetHTTPResponseBodyDataBytes( uint32_t request, uint32_t dataSize )
{
	try
	{
		return CopyBytesForC( Steam_GameServerHTTP_GetHTTPResponseBodyDataBytes( static_cast<HTTPRequestHandle>( request ), static_cast<uint32>( dataSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_Bytes SWS_Steam_GameServerHTTP_GetHTTPStreamingResponseBodyDataBytes( uint32_t request, uint32_t offset, uint32_t dataSize )
{
	try
	{
		return CopyBytesForC( Steam_GameServerHTTP_GetHTTPStreamingResponseBodyDataBytes( static_cast<HTTPRequestHandle>( request ), static_cast<uint32>( offset ), static_cast<uint32>( dataSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_GameServerHTTP_SetHTTPRequestRawPostBodyBytes( uint32_t request, const char * contentType, const uint8_t * data, size_t dataSize )
{
	try
	{
		return Steam_GameServerHTTP_SetHTTPRequestRawPostBodyBytes( static_cast<HTTPRequestHandle>( request ), contentType, data, dataSize );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_GameServerNetworkingMessages_AcceptSessionWithSteamID( uint64_t steamID )
{
	try
	{
		return Steam_GameServerNetworkingMessages_AcceptSessionWithSteamID( static_cast<uint64_steamid>( steamID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_GameServerNetworkingMessages_CloseChannelWithSteamID( uint64_t steamID, int32_t localChannel )
{
	try
	{
		return Steam_GameServerNetworkingMessages_CloseChannelWithSteamID( static_cast<uint64_steamid>( steamID ), static_cast<int>( localChannel ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_GameServerNetworkingMessages_CloseSessionWithSteamID( uint64_t steamID )
{
	try
	{
		return Steam_GameServerNetworkingMessages_CloseSessionWithSteamID( static_cast<uint64_steamid>( steamID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_BytesList SWS_Steam_GameServerNetworkingMessages_ReceiveMessagesOnChannelBytes( int32_t localChannel, int32_t maxMessages )
{
	try
	{
		return CopyBytesListForC( Steam_GameServerNetworkingMessages_ReceiveMessagesOnChannelBytes( static_cast<int>( localChannel ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_GameServerNetworkingMessages_SendMessageToSteamIDBytes( uint64_t steamID, const uint8_t * data, size_t dataSize, int32_t sendFlags, int32_t remoteChannel )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServerNetworkingMessages_SendMessageToSteamIDBytes( static_cast<uint64_steamid>( steamID ), data, dataSize, static_cast<int>( sendFlags ), static_cast<int>( remoteChannel ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_GameServerNetworkingSockets_AcceptConnection( uint32_t connection )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServerNetworkingSockets_AcceptConnection( static_cast<HSteamNetConnection>( connection ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_GameServerNetworkingSockets_CloseConnection( uint32_t connection, int32_t reason, const char * debugMessage, bool enableLinger )
{
	try
	{
		return Steam_GameServerNetworkingSockets_CloseConnection( static_cast<HSteamNetConnection>( connection ), static_cast<int>( reason ), debugMessage, enableLinger );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_GameServerNetworkingSockets_CloseListenSocket( uint32_t listenSocket )
{
	try
	{
		return Steam_GameServerNetworkingSockets_CloseListenSocket( static_cast<HSteamListenSocket>( listenSocket ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint32_t SWS_Steam_GameServerNetworkingSockets_CreateListenSocketP2PNoOptions( int32_t localVirtualPort )
{
	try
	{
		return static_cast<uint32_t>( Steam_GameServerNetworkingSockets_CreateListenSocketP2PNoOptions( static_cast<int>( localVirtualPort ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_GameServerNetworkingSockets_CreatePollGroup( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_GameServerNetworkingSockets_CreatePollGroup() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_GameServerNetworkingSockets_DestroyPollGroup( uint32_t pollGroup )
{
	try
	{
		return Steam_GameServerNetworkingSockets_DestroyPollGroup( static_cast<HSteamNetPollGroup>( pollGroup ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_BytesList SWS_Steam_GameServerNetworkingSockets_ReceiveMessagesOnConnectionBytes( uint32_t connection, int32_t maxMessages )
{
	try
	{
		return CopyBytesListForC( Steam_GameServerNetworkingSockets_ReceiveMessagesOnConnectionBytes( static_cast<HSteamNetConnection>( connection ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_GameServerNetworkingSockets_ReceiveMessagesOnConnectionStrings( uint32_t connection, int32_t maxMessages )
{
	try
	{
		return CopyStringListForC( Steam_GameServerNetworkingSockets_ReceiveMessagesOnConnectionStrings( static_cast<HSteamNetConnection>( connection ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_BytesList SWS_Steam_GameServerNetworkingSockets_ReceiveMessagesOnPollGroupBytes( uint32_t pollGroup, int32_t maxMessages )
{
	try
	{
		return CopyBytesListForC( Steam_GameServerNetworkingSockets_ReceiveMessagesOnPollGroupBytes( static_cast<HSteamNetPollGroup>( pollGroup ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_GameServerNetworkingSockets_ReceiveMessagesOnPollGroupStrings( uint32_t pollGroup, int32_t maxMessages )
{
	try
	{
		return CopyStringListForC( Steam_GameServerNetworkingSockets_ReceiveMessagesOnPollGroupStrings( static_cast<HSteamNetPollGroup>( pollGroup ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_GameServerNetworkingSockets_SendMessageToConnectionBytes( uint32_t connection, const uint8_t * data, size_t dataSize, int32_t sendFlags )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServerNetworkingSockets_SendMessageToConnectionBytes( static_cast<HSteamNetConnection>( connection ), data, dataSize, static_cast<int>( sendFlags ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_GameServerNetworkingSockets_SendMessageToConnectionString( uint32_t connection, const char * data, int32_t sendFlags )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServerNetworkingSockets_SendMessageToConnectionString( static_cast<HSteamNetConnection>( connection ), data, static_cast<int>( sendFlags ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_GameServerNetworkingSockets_SetConnectionPollGroup( uint32_t connection, uint32_t pollGroup )
{
	try
	{
		return Steam_GameServerNetworkingSockets_SetConnectionPollGroup( static_cast<HSteamNetConnection>( connection ), static_cast<HSteamNetPollGroup>( pollGroup ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_Steam_GameServer_BeginAuthSessionBytes( const uint8_t * data, size_t dataSize, uint64_t steamID )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServer_BeginAuthSessionBytes( data, dataSize, static_cast<uint64_steamid>( steamID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_GameServer_GetAuthSessionTicketBytes( int32_t maxTicketSize )
{
	try
	{
		return static_cast<uint32_t>( Steam_GameServer_GetAuthSessionTicketBytes( static_cast<int>( maxTicketSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_GameServer_GetHSteamPipe( void )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServer_GetHSteamPipe() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_GameServer_GetHSteamUser( void )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServer_GetHSteamUser() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_Bytes SWS_Steam_GameServer_GetLastAuthSessionTicketBytes( void )
{
	try
	{
		return CopyBytesForC( Steam_GameServer_GetLastAuthSessionTicketBytes() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_Steam_GameServer_GetLastInitError( void )
{
	try
	{
		return Steam_GameServer_GetLastInitError();
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_Steam_GameServer_GetLastInitResult( void )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServer_GetLastInitResult() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_GameServer_GlobalBSecure( void )
{
	try
	{
		return Steam_GameServer_GlobalBSecure();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_Steam_GameServer_GlobalGetSteamID( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_GameServer_GlobalGetSteamID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_GameServer_Init( uint32_t ip, uint16_t gamePort, uint16_t queryPort, int32_t serverMode, const char * versionString )
{
	try
	{
		return Steam_GameServer_Init( static_cast<uint32>( ip ), static_cast<uint16>( gamePort ), static_cast<uint16>( queryPort ), static_cast<int>( serverMode ), versionString );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_Steam_GameServer_InitEx( uint32_t ip, uint16_t gamePort, uint16_t queryPort, int32_t serverMode, const char * versionString )
{
	try
	{
		return static_cast<int32_t>( Steam_GameServer_InitEx( static_cast<uint32>( ip ), static_cast<uint16>( gamePort ), static_cast<uint16>( queryPort ), static_cast<int>( serverMode ), versionString ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint16_t SWS_Steam_GameServer_QueryPortShared( void )
{
	try
	{
		return static_cast<uint16_t>( Steam_GameServer_QueryPortShared() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_Steam_GameServer_ReleaseCurrentThreadMemory( void )
{
	try
	{
		Steam_GameServer_ReleaseCurrentThreadMemory();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_GameServer_RunCallbacks( void )
{
	try
	{
		Steam_GameServer_RunCallbacks();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_GameServer_Shutdown( void )
{
	try
	{
		Steam_GameServer_Shutdown();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_Steam_GetCallbackDispatchMode( void )
{
	try
	{
		return static_cast<int32_t>( Steam_GetCallbackDispatchMode() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_GetHSteamPipe( void )
{
	try
	{
		return static_cast<int32_t>( Steam_GetHSteamPipe() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_GetHSteamUser( void )
{
	try
	{
		return static_cast<int32_t>( Steam_GetHSteamUser() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_Steam_GetLastInitError( void )
{
	try
	{
		return Steam_GetLastInitError();
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

int32_t SWS_Steam_GetLastInitResult( void )
{
	try
	{
		return static_cast<int32_t>( Steam_GetLastInitResult() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

const char * SWS_Steam_GetSteamInstallPath( void )
{
	try
	{
		return Steam_GetSteamInstallPath();
	}
	catch ( const std::exception & )
	{
		return EmptyCString();
	}
}

SWS_String SWS_Steam_HTTP_GetHTTPDownloadProgressPctString( uint32_t hRequest )
{
	try
	{
		return CopyStringForC( Steam_HTTP_GetHTTPDownloadProgressPctString( static_cast<HTTPRequestHandle>( hRequest ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_HTTP_GetHTTPRequestWasTimedOutString( uint32_t hRequest )
{
	try
	{
		return CopyStringForC( Steam_HTTP_GetHTTPRequestWasTimedOutString( static_cast<HTTPRequestHandle>( hRequest ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_Bytes SWS_Steam_HTTP_GetHTTPResponseBodyDataBytes( uint32_t request, uint32_t dataSize )
{
	try
	{
		return CopyBytesForC( Steam_HTTP_GetHTTPResponseBodyDataBytes( static_cast<HTTPRequestHandle>( request ), static_cast<uint32>( dataSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_HTTP_GetHTTPResponseBodySizeString( uint32_t hRequest )
{
	try
	{
		return CopyStringForC( Steam_HTTP_GetHTTPResponseBodySizeString( static_cast<HTTPRequestHandle>( hRequest ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_Bytes SWS_Steam_HTTP_GetHTTPStreamingResponseBodyDataBytes( uint32_t request, uint32_t offset, uint32_t dataSize )
{
	try
	{
		return CopyBytesForC( Steam_HTTP_GetHTTPStreamingResponseBodyDataBytes( static_cast<HTTPRequestHandle>( request ), static_cast<uint32>( offset ), static_cast<uint32>( dataSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_HTTP_SetHTTPRequestRawPostBodyBytes( uint32_t request, const char * contentType, const uint8_t * data, size_t dataSize )
{
	try
	{
		return Steam_HTTP_SetHTTPRequestRawPostBodyBytes( static_cast<HTTPRequestHandle>( request ), contentType, data, dataSize );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Init( void )
{
	try
	{
		return Steam_Init();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_Steam_InitEx( void )
{
	try
	{
		return static_cast<int32_t>( Steam_InitEx() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_InitFlat( void )
{
	try
	{
		return static_cast<int32_t>( Steam_InitFlat() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Input_GetDeviceBindingRevisionString( uint64_t inputHandle )
{
	try
	{
		return CopyStringForC( Steam_Input_GetDeviceBindingRevisionString( static_cast<InputHandle_t>( inputHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_Inventory_GetItemDefinitionIDsList( void )
{
	try
	{
		return CopyStringListForC( Steam_Inventory_GetItemDefinitionIDsList() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Inventory_GetItemDefinitionPropertyString( int32_t definition, const char * propertyName )
{
	try
	{
		return CopyStringForC( Steam_Inventory_GetItemDefinitionPropertyString( static_cast<SteamItemDef_t>( definition ), propertyName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Inventory_GetItemPriceString( int32_t definition )
{
	try
	{
		return CopyStringForC( Steam_Inventory_GetItemPriceString( static_cast<SteamItemDef_t>( definition ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_Inventory_GetItemsWithPricesList( void )
{
	try
	{
		return CopyStringListForC( Steam_Inventory_GetItemsWithPricesList() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Inventory_GetResultItemPropertyString( int32_t resultHandle, uint32_t itemIndex, const char * propertyName )
{
	try
	{
		return CopyStringForC( Steam_Inventory_GetResultItemPropertyString( static_cast<SteamInventoryResult_t>( resultHandle ), static_cast<uint32>( itemIndex ), propertyName ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_IsSteamRunning( void )
{
	try
	{
		return Steam_IsSteamRunning();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_Steam_LobbyComparisonEqual( void )
{
	try
	{
		return static_cast<int32_t>( Steam_LobbyComparisonEqual() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_LobbyTypeFriendsOnly( void )
{
	try
	{
		return static_cast<int32_t>( Steam_LobbyTypeFriendsOnly() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_LobbyTypeInvisible( void )
{
	try
	{
		return static_cast<int32_t>( Steam_LobbyTypeInvisible() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_LobbyTypePrivate( void )
{
	try
	{
		return static_cast<int32_t>( Steam_LobbyTypePrivate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_LobbyTypePrivateUnique( void )
{
	try
	{
		return static_cast<int32_t>( Steam_LobbyTypePrivateUnique() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_LobbyTypePublic( void )
{
	try
	{
		return static_cast<int32_t>( Steam_LobbyTypePublic() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_Lobby_ChatRoomEnterResponseSuccess( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Lobby_ChatRoomEnterResponseSuccess() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_Steam_Lobby_ClearAsyncState( void )
{
	try
	{
		Steam_Lobby_ClearAsyncState();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_Lobby_ClearChatMessages( void )
{
	try
	{
		Steam_Lobby_ClearChatMessages();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint64_t SWS_Steam_Lobby_Create( int32_t lobbyType, int32_t maxMembers )
{
	try
	{
		return static_cast<uint64_t>( Steam_Lobby_Create( static_cast<int>( lobbyType ), static_cast<int>( maxMembers ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_Lobby_CreateHadIOFailure( void )
{
	try
	{
		return Steam_Lobby_CreateHadIOFailure();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Lobby_CreateSucceeded( void )
{
	try
	{
		return Steam_Lobby_CreateSucceeded();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_Steam_Lobby_EnableChatCallbacks( void )
{
	try
	{
		Steam_Lobby_EnableChatCallbacks();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_Steam_Lobby_GetCreateResult( void )
{
	try
	{
		return static_cast<int32_t>( Steam_Lobby_GetCreateResult() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_Lobby_GetCreatedLobbyID( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_Lobby_GetCreatedLobbyID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_Lobby_GetDataEntries( uint64_t lobbyID )
{
	try
	{
		return CopyStringListForC( Steam_Lobby_GetDataEntries( static_cast<uint64_steamid>( lobbyID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_Lobby_GetJoinResponse( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Lobby_GetJoinResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_Lobby_GetJoinedLobbyID( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_Lobby_GetJoinedLobbyID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_Lobby_GetListLobbyByIndex( int32_t index )
{
	try
	{
		return static_cast<uint64_t>( Steam_Lobby_GetListLobbyByIndex( static_cast<int>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Lobby_GetListLobbyNameByIndex( int32_t index )
{
	try
	{
		return CopyStringForC( Steam_Lobby_GetListLobbyNameByIndex( static_cast<int>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_Lobby_GetListResultCount( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_Lobby_GetListResultCount() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_Lobby_IsCreateComplete( void )
{
	try
	{
		return Steam_Lobby_IsCreateComplete();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Lobby_IsCreatePending( void )
{
	try
	{
		return Steam_Lobby_IsCreatePending();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Lobby_IsJoinComplete( void )
{
	try
	{
		return Steam_Lobby_IsJoinComplete();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Lobby_IsJoinPending( void )
{
	try
	{
		return Steam_Lobby_IsJoinPending();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Lobby_IsListComplete( void )
{
	try
	{
		return Steam_Lobby_IsListComplete();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Lobby_IsListPending( void )
{
	try
	{
		return Steam_Lobby_IsListPending();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

uint64_t SWS_Steam_Lobby_Join( uint64_t lobbyID )
{
	try
	{
		return static_cast<uint64_t>( Steam_Lobby_Join( static_cast<uint64_steamid>( lobbyID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_Lobby_JoinHadIOFailure( void )
{
	try
	{
		return Steam_Lobby_JoinHadIOFailure();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Lobby_JoinSucceeded( void )
{
	try
	{
		return Steam_Lobby_JoinSucceeded();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_Lobby_ListHadIOFailure( void )
{
	try
	{
		return Steam_Lobby_ListHadIOFailure();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_StringList SWS_Steam_Lobby_PollChatMessages( int32_t maxMessages )
{
	try
	{
		return CopyStringListForC( Steam_Lobby_PollChatMessages( static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_Lobby_RequestList( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_Lobby_RequestList() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_Lobby_SendChatMessage( uint64_t lobbyID, const char * message )
{
	try
	{
		return Steam_Lobby_SendChatMessage( static_cast<uint64_steamid>( lobbyID ), message );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDAvatarImageLoaded( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDAvatarImageLoaded() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDClientGameServerDeny( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDClientGameServerDeny() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDDlcInstalled( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDDlcInstalled() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDEncryptedAppTicketResponse( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDEncryptedAppTicketResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDFavoritesListChanged( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDFavoritesListChanged() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDFriendRichPresenceUpdate( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDFriendRichPresenceUpdate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDGameLobbyJoinRequested( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDGameLobbyJoinRequested() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDGameOverlayActivated( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDGameOverlayActivated() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDGameRichPresenceJoinRequested( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDGameRichPresenceJoinRequested() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDGameServerChangeRequested( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDGameServerChangeRequested() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDGameWebCallback( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDGameWebCallback() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDGetAuthSessionTicketResponse( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDGetAuthSessionTicketResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDIPCFailure( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDIPCFailure() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLicensesUpdated( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLicensesUpdated() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyChatMsg( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyChatMsg() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyChatUpdate( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyChatUpdate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyCreated( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyCreated() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyDataUpdate( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyDataUpdate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyEnter( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyEnter() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyGameCreated( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyGameCreated() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyInvite( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyInvite() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyKicked( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyKicked() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLobbyMatchList( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLobbyMatchList() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDLowBatteryPower( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDLowBatteryPower() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDMicroTxnAuthorizationResponse( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDMicroTxnAuthorizationResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDNewUrlLaunchParameters( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDNewUrlLaunchParameters() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDP2PSessionConnectFail( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDP2PSessionConnectFail() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDP2PSessionRequest( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDP2PSessionRequest() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDPersonaStateChange( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDPersonaStateChange() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSocketStatusCallback( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSocketStatusCallback() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamAPICallCompleted( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamAPICallCompleted() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamNetAuthenticationStatus( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamNetAuthenticationStatus() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamNetConnectionStatusChanged( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamNetConnectionStatusChanged() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamNetworkingFakeIPResult( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamNetworkingFakeIPResult() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamNetworkingMessagesSessionFailed( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamNetworkingMessagesSessionFailed() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamNetworkingMessagesSessionRequest( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamNetworkingMessagesSessionRequest() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamRelayNetworkStatus( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamRelayNetworkStatus() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamServerConnectFailure( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamServerConnectFailure() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamServersConnected( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamServersConnected() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamServersDisconnected( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamServersDisconnected() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDSteamShutdown( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDSteamShutdown() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_CallbackIDValidateAuthTicketResponse( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_CallbackIDValidateAuthTicketResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_ManualDispatch_CallbackIsAPICallCompleted( void )
{
	try
	{
		return Steam_ManualDispatch_CallbackIsAPICallCompleted();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeAPICallResultLobbyCreated( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeAPICallResultLobbyCreated() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeAPICallResultLobbyEnter( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeAPICallResultLobbyEnter() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeAPICallResultLobbyMatchList( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeAPICallResultLobbyMatchList() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackAvatarImageLoaded( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackAvatarImageLoaded() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackClientGameServerDeny( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackClientGameServerDeny() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackDlcInstalled( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackDlcInstalled() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackEncryptedAppTicketResponse( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackEncryptedAppTicketResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackFavoritesListChanged( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackFavoritesListChanged() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackFriendRichPresenceUpdate( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackFriendRichPresenceUpdate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackGameLobbyJoinRequested( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackGameLobbyJoinRequested() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackGameOverlayActivated( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackGameOverlayActivated() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackGameRichPresenceJoinRequested( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackGameRichPresenceJoinRequested() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackGameServerChangeRequested( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackGameServerChangeRequested() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackGameWebCallback( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackGameWebCallback() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackGetAuthSessionTicketResponse( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackGetAuthSessionTicketResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackIPCFailure( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackIPCFailure() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLicensesUpdated( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLicensesUpdated() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLobbyChatMsg( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLobbyChatMsg() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLobbyChatUpdate( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLobbyChatUpdate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLobbyDataUpdate( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLobbyDataUpdate() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLobbyEnter( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLobbyEnter() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLobbyGameCreated( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLobbyGameCreated() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLobbyInvite( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLobbyInvite() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLobbyKicked( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLobbyKicked() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackLowBatteryPower( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackLowBatteryPower() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackMicroTxnAuthorizationResponse( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackMicroTxnAuthorizationResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackNewUrlLaunchParameters( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackNewUrlLaunchParameters() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackP2PSessionConnectFail( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackP2PSessionConnectFail() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackP2PSessionRequest( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackP2PSessionRequest() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackPersonaStateChange( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackPersonaStateChange() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSocketStatusCallback( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSocketStatusCallback() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamNetAuthenticationStatus( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamNetAuthenticationStatus() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamNetConnectionStatusChanged( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamNetConnectionStatusChanged() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamNetworkingFakeIPResult( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamNetworkingFakeIPResult() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamNetworkingMessagesSessionFailed( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamNetworkingMessagesSessionFailed() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamNetworkingMessagesSessionRequest( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamNetworkingMessagesSessionRequest() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamRelayNetworkStatus( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamRelayNetworkStatus() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamServerConnectFailure( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamServerConnectFailure() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamServersConnected( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamServersConnected() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamServersDisconnected( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamServersDisconnected() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackSteamShutdown( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackSteamShutdown() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_ManualDispatch_DecodeCallbackValidateAuthTicketResponse( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_DecodeCallbackValidateAuthTicketResponse() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_Steam_ManualDispatch_FreeLastCallback( int32_t pipe )
{
	try
	{
		Steam_ManualDispatch_FreeLastCallback( static_cast<HSteamPipe>( pipe ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

bool SWS_Steam_ManualDispatch_GetAPICallResult( int32_t pipe, uint64_t apiCall, int32_t callbackSize, int32_t callbackID )
{
	try
	{
		return Steam_ManualDispatch_GetAPICallResult( static_cast<HSteamPipe>( pipe ), static_cast<SteamAPICall_t>( apiCall ), static_cast<int>( callbackSize ), static_cast<int>( callbackID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_String SWS_Steam_ManualDispatch_GetAPICallResultData( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_GetAPICallResultData() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_ManualDispatch_GetAPICallResultFailed( void )
{
	try
	{
		return Steam_ManualDispatch_GetAPICallResultFailed();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_String SWS_Steam_ManualDispatch_GetCallbackData( void )
{
	try
	{
		return CopyStringForC( Steam_ManualDispatch_GetCallbackData() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_GetCallbackID( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_GetCallbackID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_GetCallbackSize( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_GetCallbackSize() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_GetCallbackSteamUser( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_GetCallbackSteamUser() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_ManualDispatch_GetCompletedAPICall( void )
{
	try
	{
		return static_cast<uint64_t>( Steam_ManualDispatch_GetCompletedAPICall() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ManualDispatch_GetCompletedCallbackID( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ManualDispatch_GetCompletedCallbackID() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_ManualDispatch_GetCompletedCallbackSize( void )
{
	try
	{
		return static_cast<uint32_t>( Steam_ManualDispatch_GetCompletedCallbackSize() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_ManualDispatch_GetNextCallback( int32_t pipe )
{
	try
	{
		return Steam_ManualDispatch_GetNextCallback( static_cast<HSteamPipe>( pipe ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_Steam_ManualDispatch_Init( void )
{
	try
	{
		Steam_ManualDispatch_Init();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_ManualDispatch_RunFrame( int32_t pipe )
{
	try
	{
		Steam_ManualDispatch_RunFrame( static_cast<HSteamPipe>( pipe ) );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_MatchmakingServers_ClearPingResult( void )
{
	try
	{
		Steam_MatchmakingServers_ClearPingResult();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_MatchmakingServers_ClearServerFriendsResult( void )
{
	try
	{
		Steam_MatchmakingServers_ClearServerFriendsResult();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

SWS_String SWS_Steam_MatchmakingServers_GetPingServer( void )
{
	try
	{
		return CopyStringForC( Steam_MatchmakingServers_GetPingServer() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_MatchmakingServers_GetServerFriends( void )
{
	try
	{
		return CopyStringListForC( Steam_MatchmakingServers_GetServerFriends() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_MatchmakingServers_IsPingComplete( void )
{
	try
	{
		return Steam_MatchmakingServers_IsPingComplete();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_MatchmakingServers_IsPingPending( void )
{
	try
	{
		return Steam_MatchmakingServers_IsPingPending();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_MatchmakingServers_IsServerFriendsComplete( void )
{
	try
	{
		return Steam_MatchmakingServers_IsServerFriendsComplete();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_MatchmakingServers_IsServerFriendsPending( void )
{
	try
	{
		return Steam_MatchmakingServers_IsServerFriendsPending();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_MatchmakingServers_PingFailed( void )
{
	try
	{
		return Steam_MatchmakingServers_PingFailed();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_Steam_MatchmakingServers_PingServer( uint32_t ip, uint16_t port )
{
	try
	{
		return static_cast<int32_t>( Steam_MatchmakingServers_PingServer( static_cast<uint32>( ip ), static_cast<uint16>( port ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_MatchmakingServers_PingSucceeded( void )
{
	try
	{
		return Steam_MatchmakingServers_PingSucceeded();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

int32_t SWS_Steam_MatchmakingServers_ServerFriends( uint32_t ip, uint16_t port )
{
	try
	{
		return static_cast<int32_t>( Steam_MatchmakingServers_ServerFriends( static_cast<uint32>( ip ), static_cast<uint16>( port ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_MatchmakingServers_ServerFriendsFailed( void )
{
	try
	{
		return Steam_MatchmakingServers_ServerFriendsFailed();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_MatchmakingServers_ServerFriendsSucceeded( void )
{
	try
	{
		return Steam_MatchmakingServers_ServerFriendsSucceeded();
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_String SWS_Steam_Matchmaking_GetFavoriteGameString( int32_t iGame )
{
	try
	{
		return CopyStringForC( Steam_Matchmaking_GetFavoriteGameString( static_cast<int>( iGame ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Matchmaking_GetLobbyDataByIndexString( uint64_t steamIDLobby, int32_t iLobbyData )
{
	try
	{
		return CopyStringForC( Steam_Matchmaking_GetLobbyDataByIndexString( static_cast<uint64_steamid>( steamIDLobby ), static_cast<int>( iLobbyData ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetConnectionEnd_AppExceptionGeneric( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetConnectionEnd_AppExceptionGeneric() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetConnectionEnd_AppGeneric( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetConnectionEnd_AppGeneric() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingConnectionState_ClosedByPeer( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingConnectionState_ClosedByPeer() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingConnectionState_Connected( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingConnectionState_Connected() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingConnectionState_Connecting( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingConnectionState_Connecting() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingConnectionState_FindingRoute( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingConnectionState_FindingRoute() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingConnectionState_None( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingConnectionState_None() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingConnectionState_ProblemDetectedLocally( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingConnectionState_ProblemDetectedLocally() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_NetworkingMessages_AcceptSessionWithSteamID( uint64_t steamID )
{
	try
	{
		return Steam_NetworkingMessages_AcceptSessionWithSteamID( static_cast<uint64_steamid>( steamID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_NetworkingMessages_CloseChannelWithSteamID( uint64_t steamID, int32_t localChannel )
{
	try
	{
		return Steam_NetworkingMessages_CloseChannelWithSteamID( static_cast<uint64_steamid>( steamID ), static_cast<int>( localChannel ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_NetworkingMessages_CloseSessionWithSteamID( uint64_t steamID )
{
	try
	{
		return Steam_NetworkingMessages_CloseSessionWithSteamID( static_cast<uint64_steamid>( steamID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_BytesList SWS_Steam_NetworkingMessages_ReceiveMessagesOnChannelBytes( int32_t localChannel, int32_t maxMessages )
{
	try
	{
		return CopyBytesListForC( Steam_NetworkingMessages_ReceiveMessagesOnChannelBytes( static_cast<int>( localChannel ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingMessages_SendMessageToSteamIDBytes( uint64_t steamID, const uint8_t * data, size_t dataSize, int32_t sendFlags, int32_t remoteChannel )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingMessages_SendMessageToSteamIDBytes( static_cast<uint64_steamid>( steamID ), data, dataSize, static_cast<int>( sendFlags ), static_cast<int>( remoteChannel ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingSend_Reliable( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSend_Reliable() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingSend_ReliableNoNagle( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSend_ReliableNoNagle() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingSend_Unreliable( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSend_Unreliable() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingSend_UnreliableNoDelay( void )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSend_UnreliableNoDelay() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_Steam_NetworkingSockets_ClearConnectionStatusChangedEvents( void )
{
	try
	{
		Steam_NetworkingSockets_ClearConnectionStatusChangedEvents();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

uint32_t SWS_Steam_NetworkingSockets_ConnectByIPAddressStringNoOptions( const char * address )
{
	try
	{
		return static_cast<uint32_t>( Steam_NetworkingSockets_ConnectByIPAddressStringNoOptions( address ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_NetworkingSockets_ConnectP2PSteamIDNoOptions( uint64_t steamID, int32_t remoteVirtualPort )
{
	try
	{
		return static_cast<uint32_t>( Steam_NetworkingSockets_ConnectP2PSteamIDNoOptions( static_cast<uint64_steamid>( steamID ), static_cast<int>( remoteVirtualPort ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_NetworkingSockets_CreateListenSocketIPStringNoOptions( const char * address )
{
	try
	{
		return static_cast<uint32_t>( Steam_NetworkingSockets_CreateListenSocketIPStringNoOptions( address ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_NetworkingSockets_CreateListenSocketP2PFakeIPNoOptions( int32_t fakePortIndex )
{
	try
	{
		return static_cast<uint32_t>( Steam_NetworkingSockets_CreateListenSocketP2PFakeIPNoOptions( static_cast<int>( fakePortIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_NetworkingSockets_CreateListenSocketP2PNoOptions( int32_t localVirtualPort )
{
	try
	{
		return static_cast<uint32_t>( Steam_NetworkingSockets_CreateListenSocketP2PNoOptions( static_cast<int>( localVirtualPort ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_Steam_NetworkingSockets_EnableConnectionStatusCallbacks( void )
{
	try
	{
		Steam_NetworkingSockets_EnableConnectionStatusCallbacks();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetAuthenticationStatusString( void )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetAuthenticationStatusString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetConnectionInfoString( uint32_t connection )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetConnectionInfoString( static_cast<HSteamNetConnection>( connection ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetConnectionNameString( uint32_t connection )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetConnectionNameString( static_cast<HSteamNetConnection>( connection ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetConnectionRealTimeStatusString( uint32_t connection )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetConnectionRealTimeStatusString( static_cast<HSteamNetConnection>( connection ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetDetailedConnectionStatusString( uint32_t connection )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetDetailedConnectionStatusString( static_cast<HSteamNetConnection>( connection ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetFakeIPString( int32_t firstPortIndex )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetFakeIPString( static_cast<int>( firstPortIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetIdentityString( void )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetIdentityString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetListenSocketAddressString( uint32_t listenSocket )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetListenSocketAddressString( static_cast<HSteamListenSocket>( listenSocket ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_NetworkingSockets_GetRemoteFakeIPForConnectionString( uint32_t connection )
{
	try
	{
		return CopyStringForC( Steam_NetworkingSockets_GetRemoteFakeIPForConnectionString( static_cast<HSteamNetConnection>( connection ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_NetworkingSockets_PollConnectionStatusChangedStrings( int32_t maxEvents )
{
	try
	{
		return CopyStringListForC( Steam_NetworkingSockets_PollConnectionStatusChangedStrings( static_cast<int>( maxEvents ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_BytesList SWS_Steam_NetworkingSockets_ReceiveMessagesOnConnectionBytes( uint32_t connection, int32_t maxMessages )
{
	try
	{
		return CopyBytesListForC( Steam_NetworkingSockets_ReceiveMessagesOnConnectionBytes( static_cast<HSteamNetConnection>( connection ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_NetworkingSockets_ReceiveMessagesOnConnectionStrings( uint32_t connection, int32_t maxMessages )
{
	try
	{
		return CopyStringListForC( Steam_NetworkingSockets_ReceiveMessagesOnConnectionStrings( static_cast<HSteamNetConnection>( connection ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_BytesList SWS_Steam_NetworkingSockets_ReceiveMessagesOnPollGroupBytes( uint32_t pollGroup, int32_t maxMessages )
{
	try
	{
		return CopyBytesListForC( Steam_NetworkingSockets_ReceiveMessagesOnPollGroupBytes( static_cast<HSteamNetPollGroup>( pollGroup ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_StringList SWS_Steam_NetworkingSockets_ReceiveMessagesOnPollGroupStrings( uint32_t pollGroup, int32_t maxMessages )
{
	try
	{
		return CopyStringListForC( Steam_NetworkingSockets_ReceiveMessagesOnPollGroupStrings( static_cast<HSteamNetPollGroup>( pollGroup ), static_cast<int>( maxMessages ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingSockets_SendMessageToConnectionBytes( uint32_t connection, const uint8_t * data, size_t dataSize, int32_t sendFlags )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSockets_SendMessageToConnectionBytes( static_cast<HSteamNetConnection>( connection ), data, dataSize, static_cast<int>( sendFlags ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_NetworkingSockets_SendMessageToConnectionString( uint32_t connection, const char * data, int32_t sendFlags )
{
	try
	{
		return static_cast<int32_t>( Steam_NetworkingSockets_SendMessageToConnectionString( static_cast<HSteamNetConnection>( connection ), data, static_cast<int>( sendFlags ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Networking_IsDataAvailableOnSocketString( uint32_t hSocket )
{
	try
	{
		return CopyStringForC( Steam_Networking_IsDataAvailableOnSocketString( static_cast<SNetSocket_t>( hSocket ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Networking_IsDataAvailableString( uint32_t hListenSocket )
{
	try
	{
		return CopyStringForC( Steam_Networking_IsDataAvailableString( static_cast<SNetListenSocket_t>( hListenSocket ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Networking_IsP2PPacketAvailableString( int32_t nChannel )
{
	try
	{
		return CopyStringForC( Steam_Networking_IsP2PPacketAvailableString( static_cast<int>( nChannel ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Parties_GetNumAvailableBeaconLocationsString( void )
{
	try
	{
		return CopyStringForC( Steam_Parties_GetNumAvailableBeaconLocationsString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_PersonaStateAway( void )
{
	try
	{
		return static_cast<int32_t>( Steam_PersonaStateAway() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_PersonaStateBusy( void )
{
	try
	{
		return static_cast<int32_t>( Steam_PersonaStateBusy() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_PersonaStateInvisible( void )
{
	try
	{
		return static_cast<int32_t>( Steam_PersonaStateInvisible() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_PersonaStateLookingToPlay( void )
{
	try
	{
		return static_cast<int32_t>( Steam_PersonaStateLookingToPlay() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_PersonaStateLookingToTrade( void )
{
	try
	{
		return static_cast<int32_t>( Steam_PersonaStateLookingToTrade() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_PersonaStateOffline( void )
{
	try
	{
		return static_cast<int32_t>( Steam_PersonaStateOffline() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_PersonaStateOnline( void )
{
	try
	{
		return static_cast<int32_t>( Steam_PersonaStateOnline() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_PersonaStateSnooze( void )
{
	try
	{
		return static_cast<int32_t>( Steam_PersonaStateSnooze() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_Steam_ReleaseCurrentThreadMemory( void )
{
	try
	{
		Steam_ReleaseCurrentThreadMemory();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

SWS_String SWS_Steam_RemotePlay_GetSessionClientResolutionString( uint32_t unSessionID )
{
	try
	{
		return CopyStringForC( Steam_RemotePlay_GetSessionClientResolutionString( static_cast<RemotePlaySessionID_t>( unSessionID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_Bytes SWS_Steam_RemoteStorage_FileReadAsyncCompleteBytes( uint64_t readCall, uint32_t dataSize )
{
	try
	{
		return CopyBytesForC( Steam_RemoteStorage_FileReadAsyncCompleteBytes( static_cast<SteamAPICall_t>( readCall ), static_cast<uint32>( dataSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_Bytes SWS_Steam_RemoteStorage_FileReadBytes( const char * filename, int32_t dataSize )
{
	try
	{
		return CopyBytesForC( Steam_RemoteStorage_FileReadBytes( filename, static_cast<int32>( dataSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_RemoteStorage_FileWriteAsyncBytes( const char * filename, const uint8_t * data, size_t dataSize )
{
	try
	{
		return static_cast<uint64_t>( Steam_RemoteStorage_FileWriteAsyncBytes( filename, data, dataSize ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_RemoteStorage_FileWriteBytes( const char * filename, const uint8_t * data, size_t dataSize )
{
	try
	{
		return Steam_RemoteStorage_FileWriteBytes( filename, data, dataSize );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

bool SWS_Steam_RemoteStorage_FileWriteStreamWriteChunkBytes( uint64_t writeHandle, const uint8_t * data, size_t dataSize )
{
	try
	{
		return Steam_RemoteStorage_FileWriteStreamWriteChunkBytes( static_cast<UGCFileWriteStreamHandle_t>( writeHandle ), data, dataSize );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

SWS_String SWS_Steam_RemoteStorage_GetQuotaString( void )
{
	try
	{
		return CopyStringForC( Steam_RemoteStorage_GetQuotaString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_RemoteStorage_GetUGCDownloadProgressString( uint64_t hContent )
{
	try
	{
		return CopyStringForC( Steam_RemoteStorage_GetUGCDownloadProgressString( static_cast<UGCHandle_t>( hContent ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

bool SWS_Steam_RestartAppIfNecessary( uint32_t appID )
{
	try
	{
		return Steam_RestartAppIfNecessary( static_cast<AppId_t>( appID ) );
	}
	catch ( const std::exception & )
	{
		return false;
	}
}

void SWS_Steam_RunCallbacks( void )
{
	try
	{
		Steam_RunCallbacks();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

int32_t SWS_Steam_ServerModeAuthentication( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ServerModeAuthentication() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ServerModeAuthenticationAndSecure( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ServerModeAuthenticationAndSecure() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ServerModeInvalid( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ServerModeInvalid() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_ServerModeNoAuthentication( void )
{
	try
	{
		return static_cast<int32_t>( Steam_ServerModeNoAuthentication() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

void SWS_Steam_SetMiniDumpComment( const char * message )
{
	try
	{
		Steam_SetMiniDumpComment( message );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_SetTryCatchCallbacks( bool enabled )
{
	try
	{
		Steam_SetTryCatchCallbacks( enabled );
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_Shutdown( void )
{
	try
	{
		Steam_Shutdown();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

void SWS_Steam_ShutdownManualDispatch( void )
{
	try
	{
		Steam_ShutdownManualDispatch();
	}
	catch ( const std::exception & )
	{
		return;
	}
}

SWS_String SWS_Steam_UGC_GetItemDownloadInfoString( uint64_t publishedFileID )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetItemDownloadInfoString( static_cast<PublishedFileId_t>( publishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetItemInstallInfoString( uint64_t publishedFileID )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetItemInstallInfoString( static_cast<PublishedFileId_t>( publishedFileID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetItemUpdateProgressString( uint64_t updateHandle )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetItemUpdateProgressString( static_cast<UGCUpdateHandle_t>( updateHandle ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCAdditionalPreviewString( uint64_t queryHandle, uint32_t index, uint32_t previewIndex )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCAdditionalPreviewString( static_cast<UGCQueryHandle_t>( queryHandle ), static_cast<uint32>( index ), static_cast<uint32>( previewIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCKeyValueTagByIndexString( uint64_t queryHandle, uint32_t index, uint32_t tagIndex )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCKeyValueTagByIndexString( static_cast<UGCQueryHandle_t>( queryHandle ), static_cast<uint32>( index ), static_cast<uint32>( tagIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCKeyValueTagString( uint64_t handle, uint32_t index, uint32_t keyValueTagIndex )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCKeyValueTagString( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( index ), static_cast<uint32>( keyValueTagIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCKeyValueTagValueString( uint64_t queryHandle, uint32_t index, const char * key )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCKeyValueTagValueString( static_cast<UGCQueryHandle_t>( queryHandle ), static_cast<uint32>( index ), key ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCMetadataString( uint64_t queryHandle, uint32_t index )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCMetadataString( static_cast<UGCQueryHandle_t>( queryHandle ), static_cast<uint32>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCPreviewURLString( uint64_t queryHandle, uint32_t index )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCPreviewURLString( static_cast<UGCQueryHandle_t>( queryHandle ), static_cast<uint32>( index ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCStatisticString( uint64_t handle, uint32_t index, int32_t eStatType )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCStatisticString( static_cast<UGCQueryHandle_t>( handle ), static_cast<uint32>( index ), static_cast<EItemStatistic>( eStatType ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCTagDisplayNameString( uint64_t queryHandle, uint32_t index, uint32_t tagIndex )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCTagDisplayNameString( static_cast<UGCQueryHandle_t>( queryHandle ), static_cast<uint32>( index ), static_cast<uint32>( tagIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetQueryUGCTagString( uint64_t queryHandle, uint32_t index, uint32_t tagIndex )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetQueryUGCTagString( static_cast<UGCQueryHandle_t>( queryHandle ), static_cast<uint32>( index ), static_cast<uint32>( tagIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UGC_GetSupportedGameVersionDataString( uint64_t queryHandle, uint32_t index, uint32_t versionIndex )
{
	try
	{
		return CopyStringForC( Steam_UGC_GetSupportedGameVersionDataString( static_cast<UGCQueryHandle_t>( queryHandle ), static_cast<uint32>( index ), static_cast<uint32>( versionIndex ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UserStats_GetMostAchievedAchievementInfoString( void )
{
	try
	{
		return CopyStringForC( Steam_UserStats_GetMostAchievedAchievementInfoString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_UserStats_GetNextMostAchievedAchievementInfoString( int32_t iIteratorPrevious )
{
	try
	{
		return CopyStringForC( Steam_UserStats_GetNextMostAchievedAchievementInfoString( static_cast<int>( iIteratorPrevious ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

int32_t SWS_Steam_User_BeginAuthSessionBytes( const uint8_t * data, size_t dataSize, uint64_t steamID )
{
	try
	{
		return static_cast<int32_t>( Steam_User_BeginAuthSessionBytes( data, dataSize, static_cast<uint64_steamid>( steamID ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint32_t SWS_Steam_User_GetAuthSessionTicketBytes( int32_t maxTicketSize )
{
	try
	{
		return static_cast<uint32_t>( Steam_User_GetAuthSessionTicketBytes( static_cast<int>( maxTicketSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_User_GetAvailableVoiceString( uint32_t sampleRate )
{
	try
	{
		return CopyStringForC( Steam_User_GetAvailableVoiceString( static_cast<uint32>( sampleRate ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_Bytes SWS_Steam_User_GetEncryptedAppTicketBytes( int32_t maxTicketSize )
{
	try
	{
		return CopyBytesForC( Steam_User_GetEncryptedAppTicketBytes( static_cast<int>( maxTicketSize ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_Bytes SWS_Steam_User_GetLastAuthSessionTicketBytes( void )
{
	try
	{
		return CopyBytesForC( Steam_User_GetLastAuthSessionTicketBytes() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_User_GetUserDataFolderString( void )
{
	try
	{
		return CopyStringForC( Steam_User_GetUserDataFolderString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

uint64_t SWS_Steam_User_RequestEncryptedAppTicketBytes( const uint8_t * data, size_t dataSize )
{
	try
	{
		return static_cast<uint64_t>( Steam_User_RequestEncryptedAppTicketBytes( data, dataSize ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Utils_FilterTextString( int32_t context, uint64_t sourceSteamID, const char * inputMessage )
{
	try
	{
		return CopyStringForC( Steam_Utils_FilterTextString( static_cast<int>( context ), static_cast<uint64_steamid>( sourceSteamID ), inputMessage ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Utils_GetEnteredGamepadTextInputString( void )
{
	try
	{
		return CopyStringForC( Steam_Utils_GetEnteredGamepadTextInputString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Utils_GetImageSizeString( int32_t image )
{
	try
	{
		return CopyStringForC( Steam_Utils_GetImageSizeString( static_cast<int>( image ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Utils_IsAPICallCompletedString( uint64_t hSteamAPICall )
{
	try
	{
		return CopyStringForC( Steam_Utils_IsAPICallCompletedString( static_cast<SteamAPICall_t>( hSteamAPICall ) ) );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}

SWS_String SWS_Steam_Video_IsBroadcastingString( void )
{
	try
	{
		return CopyStringForC( Steam_Video_IsBroadcastingString() );
	}
	catch ( const std::exception & )
	{
		return {};
	}
}
