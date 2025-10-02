const std = @import("std");
const builtin = @import("builtin");

//*****************************************************************************
pub fn build(b: *std.Build) void
{
    // build options
    const do_strip = b.option(
        bool,
        "strip",
        "Strip the executabes"
    ) orelse false;

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    // decoder
    const librledecode = myAddStaticLibrary(b, "rledecode", target,
            optimize, do_strip);
    librledecode.linkLibC();
    librledecode.addIncludePath(b.path("."));
    librledecode.addIncludePath(b.path("src"));
    librledecode.addIncludePath(b.path("include"));
    librledecode.addCSourceFiles(.{ .files = librledecode_sources });

    b.installArtifact(librledecode);
}

//*****************************************************************************
fn myAddStaticLibrary(b: *std.Build, name: []const u8,
        target: std.Build.ResolvedTarget,
        optimize: std.builtin.OptimizeMode,
        do_strip: bool) *std.Build.Step.Compile
{
    if ((builtin.zig_version.major == 0) and (builtin.zig_version.minor < 15))
    {
        return b.addStaticLibrary(.{
            .name = name,
            .target = target,
            .optimize = optimize,
            .strip = do_strip,
        });
    }
    return b.addLibrary(.{
        .name = name,
        .root_module = b.addModule(name, .{
            .target = target,
            .optimize = optimize,
            .strip = do_strip,
        }),
        .linkage = .static,
    });
}

const librledecode_sources = &.{
    "src/rledecode.c",
};
