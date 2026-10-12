const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const steam_path = b.option(std.Build.LazyPath, "sdk_path", "Path to the Steamworks SDK root directory")
        orelse std.Build.LazyPath{.cwd_relative = "C:/Users/andpp/Downloads/steamworks_sdk_165/sdk"};
        //@panic(
        //     "Missing steam path option. See example for how to link existing steam sdk.",
        // );
    const steam_path_public = steam_path.path(b, "public");

    const steamworks_c_wrapper_module = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .link_libcpp = true,
    });
    steamworks_c_wrapper_module.addIncludePath(b.path("sw-wrappers"));
    steamworks_c_wrapper_module.addIncludePath(steam_path_public);
    steamworks_c_wrapper_module.addCSourceFiles(.{
       .root = b.path("sw-wrappers"),
       .files = &.{"steamworks_c_api.cpp", "steamworks_helpers.cpp"},
       .flags = &.{ "-std=c++17", "-fno-sanitize=undefined" },
       .language = .cpp,
    });
    const steam_works_c_lib = b.addLibrary(.{
        .name = "steamworks_c_binding",
        .linkage = .static,
        .root_module = steamworks_c_wrapper_module
    });

    const steamz_module = b.addTranslateC(.{
        .target = target,
        .optimize = optimize,
        .root_source_file = b.path("sw-wrappers/steamworks_c_api.h"),
    }).createModule();

    const mod = b.addModule("steamz", .{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
        .optimize = optimize,
        .imports = &.{
            .{ .name = "steamz", .module = steamz_module, },
        },
    });
    mod.linkLibrary(steam_works_c_lib);

    const os_tag = target.result.os.tag;
    const arch = target.result.cpu.arch;

    const lib_file = switch (os_tag) {
        .windows => "redistributable_bin/win64/steam_api64.dll",
        .linux => switch (arch) {
            .x86_64 => "redistributable_bin/linux64/libsteam_api.so",
            .aarch64 => "redistributable_bin/linuxarm64/libsteam_api.so",
            else => "redistributable_bin/linux32/libsteam_api.so",
        },
        .macos => "redistributable_bin/osx/libsteam_api.dylib",
        else => @panic("unsupported OS"),
    };
    mod.addObjectFile(steam_path.path(b, lib_file));

    const rt = b.addNamedWriteFiles("steam_runtime");
    _ = rt.addCopyFile(steam_path.path(b, lib_file), std.fs.path.basename(lib_file));

    const mod_tests = b.addTest(.{
        .root_module = mod,
    });
    const run_mod_tests = b.addRunArtifact(mod_tests);
    const test_step = b.step("test", "Run tests");
    test_step.dependOn(&run_mod_tests.step);

}

pub fn installSteamWorksRuntime(dep: *std.Build.Dependency, exe: *std.Build.Step.Compile) void {
    const b = exe.step.owner;
    const rt = dep.namedWriteFiles("steam_runtime");
    b.getInstallStep().dependOn(&b.addInstallDirectory(.{
        .source_dir = rt.getDirectory(),
        .install_dir = .bin,
        .install_subdir = "",
    }).step);
}
