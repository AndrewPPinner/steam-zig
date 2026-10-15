const std = @import("std");
const steam = @import("steamz");

pub const Common = @import("common.zig");

pub const Server = struct {
    pub fn init() !Server {}
    pub fn deinit(_: *Server) void {}

    pub fn validateOwnership(_: *Server, user_id: u64, app_id: u32) bool {
        return steam.SWS_SteamAPI_ISteamGameServer_UserHasLicenseForApp(user_id, app_id) == 0;
    }
};
