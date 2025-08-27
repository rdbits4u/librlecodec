const std = @import("std");

pub fn build(b: *std.Build) void {
    // build options
    const do_strip = b.option(
        bool,
        "strip",
        "Strip the executabes"
    ) orelse false;

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    // decoder
    const librledecode = b.addStaticLibrary(.{
        .name = "rledecode",
        .target = target,
        .optimize = optimize,
        .strip = do_strip,
    });
    librledecode.linkLibC();
    librledecode.addIncludePath(b.path("."));
    librledecode.addIncludePath(b.path("src"));
    librledecode.addIncludePath(b.path("include"));
    librledecode.addCSourceFiles(.{ .files = librledecode_sources });

    b.installArtifact(librledecode);
}

const librledecode_sources = &.{
    "src/rledecode.c",
};
