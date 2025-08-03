const std = @import("std");
const builtin = @import("builtin");
var optimize: std.builtin.OptimizeMode = undefined;

fn win_run_qemu_rel(b: *std.Build) *std.Build.Step {
    return &b.addSystemCommand(&.{
        "C:\\Program Files\\qemu\\qemu-system-x86_64.exe","-bios","OVMF.fd","-serial","stdio","-d","cpu_reset","-drive","file=fat:rw:disk,format=raw",
    }).step;
}

fn win_copy_file(b: *std.Build, file: []const u8, dest: []const u8) *std.Build.Step {
    return &b.addSystemCommand(&.{
        "cmd.exe",
        "/C",
        "COPY",
        file,
        dest,
    }).step;
}

fn copy_file(b: *std.Build, file: []const u8, dest: []const u8) *std.Build.Step {
    return &b.addSystemCommand(&.{
        "mcopy",
        "-i",
        "nvme.img",
        file,
        dest,
        "-o",
    }).step;
}

fn run_qemu_rel(b: *std.Build) *std.Build.Step {
    return &b.addSystemCommand(&.{
        "qemu-system-x86_64",
        "-m",
        "512M",
        "-bios",
        "/usr/share/ovmf/OVMF.fd",
        "-drive",
        "format=raw,file=nvme.img",
        "-serial",
        "stdio",
        "-d",
        "cpu_reset",
        "-display",
        "none",
    }).step;
}

fn run_qemu_debug(b: *std.Build) *std.Build.Step {
    return &b.addSystemCommand(&.{
        "qemu-system-x86_64",
        "-s",
        "-S",
        "-m",
        "512M",
        "-bios",
        "/usr/share/ovmf/OVMF.fd",
        "-drive",
        "format=raw,file=nvme.img",
        "-serial",
        "stdio",
        "-d",
        "cpu_reset",
    }).step;
}

fn build_uefi(b: *std.Build) *std.Build.Step {
    const target_query = std.Target.Query{
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

    const step_uefi_install = b.addInstallArtifact(uefi, .{});
    return &step_uefi_install.step;
}

fn build_kernel(b: *std.Build) *std.Build.Step {
    var disabled_features = std.Target.Cpu.Feature.Set.empty;
    var enabled_features = std.Target.Cpu.Feature.Set.empty;
    disabled_features.addFeature(@intFromEnum(std.Target.x86.Feature.mmx));
    disabled_features.addFeature(@intFromEnum(std.Target.x86.Feature.sse));
    disabled_features.addFeature(@intFromEnum(std.Target.x86.Feature.sse2));
    disabled_features.addFeature(@intFromEnum(std.Target.x86.Feature.avx));
    disabled_features.addFeature(@intFromEnum(std.Target.x86.Feature.avx2));
    enabled_features.addFeature(@intFromEnum(std.Target.x86.Feature.soft_float));

    const target_query = std.Target.Query{
        .cpu_arch = std.Target.Cpu.Arch.x86_64,
        .os_tag = std.Target.Os.Tag.freestanding,
        .abi = std.Target.Abi.none,
        .cpu_features_sub = disabled_features,
        .cpu_features_add = enabled_features,
    };

    const kernel = b.addExecutable(.{
        .name = "Kernel.elf",
        .root_source_file = b.path("kernel/main.zig"),
        .target = b.resolveTargetQuery(target_query),
        .optimize = optimize,
        .code_model = .large,
    });

    kernel.setLinkerScript(b.path("kernel/linker.ld"));

    const step_kernel_install = b.addInstallArtifact(kernel, .{});
    return &step_kernel_install.step;
}

pub fn build(b: *std.Build) void {
    optimize = b.standardOptimizeOption(.{});
    const step_build_uefi = build_uefi(b);
    const step_build_kernel = build_kernel(b);
    const step_build_all = b.step("build_all", "Creates the uefi image and run the kernel");
    step_build_all.dependOn(step_build_uefi);
    step_build_all.dependOn(step_build_kernel);

    if (builtin.os.tag == .linux) {
        const step_copy_boot = copy_file(b, "./zig-out/bin/bootx64.efi", "::/EFI/BOOT");
        step_copy_boot.dependOn(step_build_uefi);

        const step_copy_kernel = copy_file(b, "./zig-out/bin/Kernel.elf", "::/");
        step_copy_kernel.dependOn(step_build_kernel);
        
        const step_copy_all = b.step("copy_all", "Copys all");
        step_copy_all.dependOn(step_copy_boot);
        step_copy_all.dependOn(step_copy_kernel);

        const run_rel_qemu = run_qemu_rel(b, "format=raw,file=nvme.img");
        run_rel_qemu.dependOn(step_copy_all);

        const release = b.step("release", "Creates the uefi image and run the kernel");
        release.dependOn(run_rel_qemu);

        const run_debug_qemu = run_qemu_debug(b);
        run_debug_qemu.dependOn(step_copy_all);

        const debug = b.step("debug", "Creates the uefi image and run the kernel");
        debug.dependOn(run_debug_qemu);
    } else {
        const step_copy_boot = win_copy_file(b, "zig-out\\bin\\bootx64.efi", "disk\\EFI\\BOOT");
        step_copy_boot.dependOn(step_build_uefi);

        const step_copy_kernel = win_copy_file(b, "zig-out\\bin\\Kernel.elf", "disk");
        step_copy_kernel.dependOn(step_build_kernel);

        const step_copy_all = b.step("copy_all", "Copys all");
        step_copy_all.dependOn(step_copy_boot);
        step_copy_all.dependOn(step_copy_kernel);

        const run_rel_qemu = win_run_qemu_rel(b);
        run_rel_qemu.dependOn(step_copy_all);

        const release = b.step("release", "Creates the uefi image and run the kernel");
        release.dependOn(run_rel_qemu);

        //const run_debug_qemu = run_qemu_debug(b);
        //run_debug_qemu.dependOn(step_copy_all);
        //
        //const debug = b.step("debug", "Creates the uefi image and run the kernel");
        //debug.dependOn(run_debug_qemu);        
    }
}
