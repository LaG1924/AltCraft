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
	local uiMainMenu = con:LoadDocument("altcraft/ui/main-menu")
	con:LoadDocument("altcraft/ui/hud")
	con:LoadDocument("altcraft/ui/pause")
	con:LoadDocument("altcraft/ui/options")
	con:LoadDocument("altcraft/ui/loading")
	con:LoadDocument("altcraft/ui/respawn")

	uiMainMenu:Show()
	AC.Settings.Load()
	uiMainMenu:GetElementById("username"):SetAttribute("value", AC.Settings.Read("username","Username"..tostring(math.random(10000))))
	uiMainMenu:GetElementById("hostname"):SetAttribute("value",AC.Settings.Read("hostname","127.0.0.1"))
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

require("altcraft/ui")

function plugin.onTick (deltaTime)
	UpdateUi()
	if AC.GetGameState() and AC.GetGameState():GetPlayer() and AC.GetGameState():GetTimeStatus().worldAge > 0 then
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