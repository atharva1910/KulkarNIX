const std = @import("std");
const PMemManager = @import("pmem.zig").PMemManager;
const KError = @import("kerrors.zig").KError;
const PageTableMgr = @import("paging.zig").PageTableMgr;
//const PMemNode = @import("pmem.zig").PMemNode;
var KnixState: KState = undefined;

pub const KState = struct {
    PMemMgr: ?*PMemManager,
    PTManager: ?*PageTableMgr,

    pub fn GetPhyMemMgr() ?*PMemManager {
        return KnixState.PMemMgr;
    }

    pub fn SetPageTableManger(mgr: *PageTableMgr) void {
        KnixState.PTManager = mgr;
    }

    pub fn GetPageTableManger() ?*PageTableMgr {
        return KnixState.PTManager;
    }
};
