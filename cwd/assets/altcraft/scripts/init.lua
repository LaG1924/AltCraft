local plugin = {
	name = 'altcraft',
	displayName = "AltCraft Core Plugin",
	onLoad = nil,
	onUnload = nil,
	onChangeState = nil,
	onTick = nil,
	onRequestBlockInfo = nil,
}

function plugin.onLoad ()
	rmlui:LoadFontFace("altcraft/fonts/OpenSans-Regular")
	local con = rmlui.contexts["default"]
	con:LoadDocument("altcraft/ui/main-menu"):Show()
	con:LoadDocument("altcraft/ui/hud")
	con:LoadDocument("altcraft/ui/pause")
end

function plugin.onChangeState (newState)
	local toHide = {}
	local toShow = {}

	for i,doc in ipairs(rmlui.contexts["default"].documents) do
		if doc.title == newState then
			toShow[#toShow+1]=doc
		else
			toHide[#toHide+1]=doc
		end
    end

	for i,doc in ipairs(toHide) do
		doc:Hide()
	end

	for i,doc in ipairs(toShow) do
		doc:Show()
	end
end

function plugin.onUnload ()
	AC.LogInfo("AC Core unloaded")
end

local function UpdateHud()
	local doc = {}
	for i,d in ipairs(rmlui.contexts["default"].documents) do
		if d.title == "Playing" then
			doc = d
		end
    end

	local playerEnt = AC.GetGameState():GetPlayer()
	doc:GetElementById('dbg-pos').inner_rml = string.format("%.1f %.1f %.1f", playerEnt.pos.x, playerEnt.pos.y, playerEnt.pos.z)
	
	local selection = AC.GetGameState():GetSelectionStatus()
	if selection.isBlockSelected then
		doc:GetElementById('dbg-select-pos').inner_rml = tostring(selection.selectedBlock)
	else
		doc:GetElementById('dbg-select-pos').inner_rml = ""
	end

	local player = AC.GetGameState():GetPlayerStatus()
	local playerHp = string.format("%.0f", player.health)
	doc:GetElementById('status-hp').inner_rml = playerHp
	doc:GetElementById('status-hp-bar'):SetAttribute("value", playerHp)
end

function plugin.onTick (deltaTime)
	if AC.GetGameState() and AC.GetGameState():GetPlayer() and AC.GetGameState():GetTimeStatus().worldAge > 0 then
		UpdateHud()
		-- local player = AC.GetGameState():GetPlayer()
		-- player.pos.x = player.pos.x + deltaTime * 0.5

		-- local playerPos = AC.GetGameState():GetPlayer().pos
		-- local wrld = AC.GetGameState():GetWorld()
		-- playerPosV = Vector.new(playerPos.x, playerPos.y - 1, playerPos.z)
		-- bid = wrld:GetBlockId(playerPosV)
		-- print(bid.id..":"..bid.state)
	end
end

local blocks = require("altcraft/blocks")
blocks.RegisterBlocks()

function plugin.onRequestBlockInfo(blockPos)
	return blocks.GetBlockInfo(blockPos)
end

AC.RegisterDimension(0, Dimension.new("overworld", true))
AC.RegisterDimension(-1, Dimension.new("the_nether", false))
AC.RegisterDimension(1, Dimension.new("the_end", false))

AC.RegisterPlugin(plugin)
plugin = nil