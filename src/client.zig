const std = @import("std");
const steam = @import("steamz");

pub const Common = @import("common.zig");

pub const Client = struct {
    pub fn init() !Client {
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

    pub fn getAuthSessionTicket(_: *Client) AuthTicket {
        const handle = steam.SWS_Steam_User_GetAuthSessionTicketBytes(1024);
        const data = steam.SWS_Steam_User_GetLastAuthSessionTicketBytes();
        return AuthTicket.init(data, handle);
    }

    pub fn getAuthWebTicket(_: *Client, service_identity: [*c]const u8) void {
        steam.SWS_SteamAPI_ISteamUser_GetAuthTicketForWebApi(service_identity);
    }

    pub fn hasFriend(_: *Client, friend_id: u64) bool {
        return steam.SWS_SteamAPI_ISteamFriends_HasFriend(friend_id, steam.SWS_Steam_FriendFlagImmediate());
    }

    pub fn deinit(_: *Client) void {
        steam.SWS_Steam_Shutdown();
    }
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
