const std = @import("std");
const builtin = @import("builtin");
var optimize: std.builtin.OptimizeMode = undefined;

const os_interface = struct {
    qemu_rel: *const fn (b: *std.Build) *std.Build.Step,
    qemu_debug: *const fn (b: *std.Build) *std.Build.Step,
    copy: *const fn (b: *std.Build, file: []const u8, dest: []const u8) *std.Build.Step,

    fn init(qemu_rel: fn (b: *std.Build) *std.Build.Step, qemu_debug: fn (b: *std.Build) *std.Build.Step, copy: fn (b: *std.Build, file: []const u8, dest: []const u8) *std.Build.Step) os_interface {
        return .{
            .qemu_rel = qemu_rel,
            .qemu_debug = qemu_debug,
            .copy = copy,
        };
    }
};

const windows = struct {
    fn run_qemu_rel(b: *std.Build) *std.Build.Step {
        return &b.addSystemCommand(&.{
            "C:\\Program Files\\qemu\\qemu-system-x86_64.exe",
            "-bios",
            "OVMF.fd",
            "-serial",
            "stdio",
            "-d",
            "cpu_reset",
            "-drive",
            "file=fat:rw:disk,format=raw",
            "-display",
            "none",
        }).step;
    }

    fn run_qemu_debug(b: *std.Build) *std.Build.Step {
        return &b.addSystemCommand(&.{
            "C:\\Program Files\\qemu\\qemu-system-x86_64.exe", "-bios", "OVMF.fd", "-serial", "stdio", "-d", "cpu_reset", "-drive", "file=fat:rw:disk,format=raw",
        }).step;
    }

    fn copy_file(b: *std.Build, file: []const u8, dest: []const u8) *std.Build.Step {
        return &b.addSystemCommand(&.{
            "cmd.exe",
            "/C",
            "COPY",
            file,
            dest,
        }).step;
    }
};

const linux = struct {
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
            "-bios",
            "/usr/share/OVMF/OVMF_CODE.fd",
            "-drive",
            "format=raw,file=nvme.img",
            "-serial",
            "stdio",
            "-d",
            "cpu_reset",
            "-display",
            "none",
            "-m",
            "4G",
        }).step;
    }

    fn run_qemu_debug(b: *std.Build) *std.Build.Step {
        return &b.addSystemCommand(&.{
            "qemu-system-x86_64",
            "-s",
            "-S",
            "-bios",
            "/usr/share/OVMF/OVMF_CODE.fd",
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
};

fn build_uefi(b: *std.Build) *std.Build.Step {
    const target_query = std.Target.Query{
        .cpu_arch = std.Target.Cpu.Arch.x86_64,
        .os_tag = std.Target.Os.Tag.uefi,
        .abi = std.Target.Abi.msvc,
    };

    const uefi_module = b.addModule("uefi_module", .{
        .root_source_file = b.path("boot/main.zig"),
        .target = b.resolveTargetQuery(target_query),
        .optimize = optimize,
    });

    const uefi = b.addExecutable(.{
        .name = "bootx64",
        .root_module = uefi_module,
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

    const kernel_module = b.addModule("kernel_module", .{
        .root_source_file = b.path("kernel/main.zig"),
        .target = b.resolveTargetQuery(target_query),
        .optimize = optimize,
        .code_model = .large,
    });

    const kernel = b.addExecutable(.{
        .name = "Kernel.elf",
        .root_module = kernel_module,
        .use_llvm = true,
    });

    kernel.setLinkerScript(b.path("kernel/linker.ld"));

    const step_kernel_install = b.addInstallArtifact(kernel, .{});
    return &step_kernel_install.step;
}

pub fn build(b: *std.Build) void {
    var os: os_interface = undefined;

    if (builtin.os.tag == .linux) {
        os = os_interface.init(linux.run_qemu_rel, linux.run_qemu_debug, linux.copy_file);
    } else {
        os = os_interface.init(windows.run_qemu_rel, windows.run_qemu_debug, windows.copy_file);
    }

    optimize = b.standardOptimizeOption(.{});
    const step_build_uefi = build_uefi(b);
    const step_build_kernel = build_kernel(b);
    const step_build_all = b.step("build_all", "Creates the uefi image and run the kernel");
    step_build_all.dependOn(step_build_uefi);
    step_build_all.dependOn(step_build_kernel);

    var file: []const u8 = undefined;
    var dest: []const u8 = undefined;

    if (builtin.os.tag == .linux) {
        file = "zig-out/bin/bootx64.efi";
        dest = "::/EFI/BOOT";
    } else {
        file = "zig-out\\bin\\bootx64.efi";
        dest = "disk\\EFI\\BOOT";
    }

    const step_copy_boot = os.copy(b, file, dest);
    step_copy_boot.dependOn(step_build_uefi);

    if (builtin.os.tag == .linux) {
        file = "zig-out/bin/Kernel.elf";
        dest = "::/";
    } else {
        file = "zig-out\\bin\\Kernel.elf";
        dest = "disk\\";
    }

    const step_copy_kernel = os.copy(b, file, dest);
    step_copy_kernel.dependOn(step_build_kernel);

    const step_copy_all = b.step("copy_all", "Copys all");
    step_copy_all.dependOn(step_copy_boot);
    step_copy_all.dependOn(step_copy_kernel);

    const run_rel_qemu = os.qemu_rel(b);
    run_rel_qemu.dependOn(step_copy_all);

    const release = b.step("release", "Creates the uefi image and run the kernel");
    release.dependOn(run_rel_qemu);

    const run_debug_qemu = os.qemu_debug(b);
    run_debug_qemu.dependOn(step_copy_all);

    const debug = b.step("debug", "Creates the uefi image and run the kernel");
    debug.dependOn(run_debug_qemu);
}
