const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});
    var target_query = std.Target.Query{
        .cpu_arch = std.Target.Cpu.Arch.x86_64,
        .os_tag = std.Target.Os.Tag.uefi,
        .abi = std.Target.Abi.msvc,
    };

    const uefi = b.addExecutable(.{
        .name = "bootx64",
        .root_source_file = b.path("boot/main.zig"),
        .target = b.resolveTargetQuery(target_query),
        .optimize = optimize,
    });

    const uefi_install = b.addInstallArtifact(uefi, .{});

    target_query = std.Target.Query{
        .cpu_arch = std.Target.Cpu.Arch.x86_64,
        .os_tag = std.Target.Os.Tag.freestanding,
        .abi = std.Target.Abi.none,
    };

    const step_build_uefi = b.step("uefi", "Builds only uefi");
    step_build_uefi.dependOn(&uefi_install.step);

    const kernel = b.addExecutable(.{
        .name = "Kernel.elf",
        .root_source_file = b.path("kernel/main.zig"),
        .target = b.resolveTargetQuery(target_query),
        .optimize = optimize,
        .code_model = .large,
    });

    kernel.setLinkerScript(b.path("kernel/linker.ld"));
    const kernel_install = b.addInstallArtifact(kernel, .{});
    const step_build_kernel = b.step("kernel", "Builds only kernel");
    step_build_kernel.dependOn(&kernel_install.step);

    b.default_step.dependOn(step_build_kernel);
    b.default_step.dependOn(step_build_uefi);

    const run = b.step("run", "Creates the uefi image and runs the kernel");
    run.dependOn(step_build_uefi);
    run.dependOn(step_build_kernel);
}
