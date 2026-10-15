const std = @import("std");
const steam = @import("steamz");
const common = @import("common.zig");

pub const SteamClientErrors = error{ SteamRestartRequired, StreamInitError };
pub const SteamClient = struct {
    pub fn init() SteamClientErrors!SteamClient {
        const app_id = steam.SWS_SteamAPI_ISteamUtils_GetAppID();
        if (!steam.SWS_Steam_RestartAppIfNecessary(app_id)) {
            std.log.debug("Steam Restart Required", .{});
            // return error.SteamRestartRequired;
        }

        const init_code = steam.SWS_Steam_InitEx();
        if (init_code != 0) {
            const error_text = steam.SWS_Steam_GetLastInitError();
            std.log.debug("steam init error: {s}", .{error_text});
            return error.StreamInitError;
        }

        return .{};
    }

    pub fn getAuthSessionTicket(_: *SteamClient) AuthTicket {
        const handle = steam.SWS_Steam_User_GetAuthSessionTicketBytes(1024);
        const data = steam.SWS_Steam_User_GetLastAuthSessionTicketBytes();
        return AuthTicket.init(data, handle);
    }

    pub fn getFriendCount(_: *SteamClient) i32 {
        const flag = steam.SWS_Steam_FriendFlagAll();
        return steam.SWS_SteamAPI_ISteamFriends_GetFriendCount(flag);
    }

    pub fn getFriendIdByIndex(_: *SteamClient, index: i32) u64 {
        const flag = steam.SWS_Steam_FriendFlagAll();
        return steam.SWS_SteamAPI_ISteamFriends_GetFriendByIndex(index, flag);
        // return steam.SWS_Steam_Friends_GetFriendByIndexImmediate(index);
    }

    pub fn getFriendDetailById(_: *SteamClient, friend_id: u64) FriendDetail {
        const name = steam.SWS_SteamAPI_ISteamFriends_GetFriendPersonaName(friend_id);
        const status = steam.SWS_SteamAPI_ISteamFriends_GetFriendPersonaState(friend_id);
        const game_id = steam.SWS_Steam_Friends_GetFriendGameID(friend_id);

        return .{
            .friend_status = status,
            .persona_name = name,
            .game_id = game_id,
        };
    }

    //This likely can't be a const going forward since I will want to track overlay state, friends, etc via these functions
    pub fn openInviteOverlay(_: *const SteamClient, connect_command: [*c]const u8) void {
        steam.SWS_SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialogConnectString(connect_command);
    }

    // Need to confirm how this looks to the user and how this gets passed to the launch commands of the game
    // I need to encode certain things in the launch command (lobby_code and random password if invite only || Can pull invite only out of DB)
    pub fn inviteFriend(_: *SteamClient, friend_id: u64, connect_command: [*c]const u8) bool {
        return steam.SWS_SteamAPI_ISteamFriends_InviteUserToGame(friend_id, connect_command);
    }

    pub fn getFriendQueryBuilder(self: *SteamClient) void {
        _ = self.getFriendCount();
        //Return a filtered down version of friends based on query struct (status, name, game, etc) caching the entire friends list on the client or friend query builder
    }

    // Ensure generated header files has an enum for ALL callback ids
    // Make a zig manual dispatch loop since we can only support manual dispatch in zig (based on IO and or a loop that stacks up queue as dict and can be pulled from by ID)
    pub fn getAuthWebTicket(_: *SteamClient, service_identity: [*c]const u8) u32 {
        steam.SWS_Steam_ManualDispatch_Init();
        const handle = steam.SWS_SteamAPI_ISteamUser_GetAuthTicketForWebApi(service_identity);

        while (true) {
            const pipe = steam.SWS_Steam_GetHSteamPipe();
            steam.SWS_Steam_ManualDispatch_RunFrame(pipe);

            while (steam.SWS_Steam_ManualDispatch_GetNextCallback(pipe)) {
                defer steam.SWS_Steam_ManualDispatch_FreeLastCallback(pipe);
                const callback_id = steam.SWS_Steam_ManualDispatch_GetCallbackID();
                std.log.debug("callback {}", .{callback_id});
            }
        }

        return handle;
    }

    pub fn hasFriend(_: *SteamClient, friend_id: u64) bool {
        return steam.SWS_SteamAPI_ISteamFriends_HasFriend(friend_id, steam.SWS_Steam_FriendFlagImmediate());
    }

    pub fn deinit(_: *SteamClient) void {
        steam.SWS_Steam_Shutdown();
    }
};

pub const FriendDetail = struct {
    persona_name: [*c]const u8,
    friend_status: i32,
    game_id: u64,
};

pub const AuthTicket = struct {
    handle: u32,
    data: steam.struct_SWS_Bytes,

    pub fn init(data: steam.struct_SWS_Bytes, auth_handle: u32) AuthTicket {
        return .{
            .data = data,
            .handle = auth_handle,
        };
    }

    pub fn deinit(self: *AuthTicket) void {
        steam.SWS_SteamAPI_ISteamUser_CancelAuthTicket(self.handle);
        self.data.SWS_FreeBytes();
    }
};
