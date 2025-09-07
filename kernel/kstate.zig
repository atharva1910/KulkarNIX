const std = @import("std");
const PMemManager = @import("pmem.zig").PMemManager;
const KError = @import("kerrors.zig").KError;
const PageTableMgr = @import("paging.zig").PageTableMgr;
const HeapManager = @import("heap.zig").HeapManager;
//const PMemNode = @import("pmem.zig").PMemNode;
var KnixState: KState = undefined;

pub const KState = struct {
    PMemMgr: ?*PMemManager,
    HeapMgr: ?*HeapManager,
    PTManager: ?*PageTableMgr,

    pub fn SetPhyMemMgr(mgr: *PMemManager) void {
        KnixState.PMemMgr = mgr;
    }

    pub fn GetPhyMemMgr() ?*PMemManager {
        return KnixState.PMemMgr;
    }

    pub fn SetPageTableManger(mgr: *PageTableMgr) void {
        KnixState.PTManager = mgr;
    }

    pub fn GetPageTableManger() ?*PageTableMgr {
        return KnixState.PTManager;
    }

    pub fn SetHeapManager(mgr: *HeapManager) void {
        KnixState.HeapMgr = mgr;
    }

    pub fn GetHeapManager() ?*HeapManager {
        return KnixState.HeapMgr;
    }
};
