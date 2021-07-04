local plugin = {
	name = 'altcraft',
	displayName = "AltCraft Core Plugin",
	onLoad = nil,
	onUnload = nil,
	onChangeState = nil,
	onTick = nil,
	onRequestBlockInfo = nil,
	onChatMessage = nil,
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
	con:LoadDocument("altcraft/ui/chat")

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

function plugin.onChatMessage(chat, pos)
	local chatDoc = {}
	for i,d in ipairs(rmlui.contexts["default"].documents) do
		if d.title == "Chat" then
			chatDoc = d
		end
    end

	local msg = chat:ToPlainText()
	msg = string.gsub(msg,'&','&amp;')
	msg = string.gsub(msg,'<','&lt;')
	msg = string.gsub(msg,'>','&gt;')
	msg = string.gsub(msg,'""','&quot;')
	msg = string.gsub(msg,"''",'&apos;')

	local color = ""
	if pos == 0 then
		color = ""
	elseif pos == 1 then
		color = 'style="color: #BBBBBB"'
	elseif pos == 2 then
		color = 'style="color: maroon"'
	else
		color = 'style="color: navy"'
	end

	chatDoc:GetElementById('chat').inner_rml = chatDoc:GetElementById('chat').inner_rml .. string.format('<p class="chat-msg" %s>%s</p>', color, msg)
	MoveChatToBottom = true
end

AC.RegisterDimension(0, Dimension.new("overworld", true))
AC.RegisterDimension(-1, Dimension.new("the_nether", false))
AC.RegisterDimension(1, Dimension.new("the_end", false))

AC.RegisterPlugin(plugin)
plugin = nil
