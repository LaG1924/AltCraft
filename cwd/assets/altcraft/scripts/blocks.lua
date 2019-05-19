local function GetBlockInfo(blockPos)
    print("Request data for "..blockPos)
    local bi = BlockInfo.new()
    bi.collides = true
    bi.blockstate = ""
    bi.variant = ""
    return bi
end

local function RegisterBlocks()
    AC.RegisterBlock(BlockId.new(1,0), true, "stone", "normal")
    AC.RegisterBlock(BlockId.new(2,0), true, "grass", "snowy=false")
    AC.RegisterBlock(BlockId.new(3,0), true, "dirt", "normal")
    AC.RegisterBlock(BlockId.new(31,1), false, "tall_grass", "normal")
end

return {
    GetBlockInfo = GetBlockInfo,
    RegisterBlocks = RegisterBlocks,
}