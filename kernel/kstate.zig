const std = @import("std");
const PMemManager = @import("pmem.zig").PMemManager;
const KError = @import("kerrors.zig").KError;
//const PMemNode = @import("pmem.zig").PMemNode;
var KnixState: KState = undefined;

pub const KState = struct {
    PMemMgr: ?*PMemManager,

    pub fn Init(kMemAddr: usize) !void {
        if (kMemAddr == 0) {
            return KError.Failed;
        }

        KnixState = .{
            .PMemMgr = @ptrFromInt(kMemAddr),
        };
    }

    pub fn GetGlobalState() KState {
        return KnixState;
    }

    pub fn GetPhyMemMgr() ?*PMemManager {
        return KnixState.PMemMgr;
    }
};
