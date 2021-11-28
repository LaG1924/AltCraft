local options = {
	gamma = 2.2,
	flight = false,
	mouseSensetivity = 0.1,
	renderDistance = 2,
	resolutionScale = 1.0,
	targetFps = 60,
	vsync = false,
	wireframe = false
}

function OpenOptions(doc)
    optionsReturnDocument = doc
    local optionsDoc = {}
    for i,d in ipairs(rmlui.contexts["default"].documents) do
        if d.title == "Options" then
			optionsDoc = d
		end
    end
    doc:Hide()
    optionsDoc:Show()
	optionsDoc.style["background-color"] = doc.style["background-color"]
end

function CloseOptions(doc)
	for i, v in pairs(options) do
		local input = doc:GetElementById(i)
		if type(v) == "number" then
			local val = input:GetAttribute("value")
			if i == "targetFps" and val == 301 then
				AC.Settings.WriteDouble(i, 10000)
			else
				AC.Settings.WriteDouble(i, tonumber(val))
			end
		elseif type(v) == "boolean" then
			local val = input:HasAttribute("checked")
			AC.Settings.WriteBool(i, val)
		end
	end
	AC.Settings.Save()
	AC.SettingsUpdate()

    optionsReturnDocument:Show()
    doc:Hide()
end

function ConnectToServer(doc)
	AC.Settings.Write('hostname',doc:GetElementById('hostname'):GetAttribute('value'))
    AC.Settings.Write('username',doc:GetElementById('username'):GetAttribute('value'))
    AC.Settings.Save()
    AC.ConnectToServer(
		doc:GetElementById('hostname'):GetAttribute('value'),
		doc:GetElementById('username'):GetAttribute('value'))
end

function SendChatMessage(doc)
	local msg = doc:GetElementById("chat-input"):GetAttribute("value")
	if msg == nil then
		return
	end
	doc:GetElementById("chat-input"):SetAttribute("value", "")

	AC.SendChatMessage(msg)
end

function OptionsDefaultHandler(event)
	local input = event.current_element.previous_sibling
	local id = input:GetAttribute("id")
	if input:GetAttribute("type") == "checkbox" then
		if options[id] then
			input:SetAttribute("checked", "")
		else
			input:RemoveAttribute("checked")
		end
	else
		input:SetAttribute("value", options[id])
	end
end

local lastFps = {}

local function UpdateFps(newFps)
	lastFps[#lastFps + 1] = newFps
	if #lastFps >= 100 then
		table.remove(lastFps, 1)
	end
	local smoothFps = 0
	for i,v in ipairs(lastFps) do
		smoothFps = smoothFps + v
	end
	smoothFps = smoothFps / #lastFps
	return smoothFps
end

function UpdateUi()
	local doc = {}
	local uiDoc = {}
	local chatDoc = {}
	for i,d in ipairs(rmlui.contexts["default"].documents) do
		if d.title == "Playing" then
			doc = d
		elseif d.title == "Options" then
			uiDoc = d
		elseif d.title == "Chat" then
			chatDoc = d
		end
    end

	if MoveChatToBottom ~= nil and MoveChatToBottom == true then
		chatDoc:GetElementById('chat').scroll_top = chatDoc:GetElementById('chat').scroll_height
	end

	if AC.GetGameState() and AC.GetGameState():GetPlayer() and AC.GetGameState():GetTimeStatus().worldAge > 0 then
		local time = AC.GetTime()
		local rawFps = 1.0 / time:GetRealDeltaS()
		local smoothFps = UpdateFps(rawFps)
		doc:GetElementById('dbg-fps').inner_rml = string.format("%.1f", smoothFps)

		local playerEnt = AC.GetGameState():GetPlayer()				
		doc:GetElementById('dbg-pos').inner_rml = string.format("%.1f %.1f %.1f", playerEnt.pos.x, playerEnt.pos.y, playerEnt.pos.z)

		local wrld = AC.GetGameState():GetWorld()
		local selection = AC.GetGameState():GetSelectionStatus()
		if selection.isBlockSelected then
			bid = wrld:GetBlockId(selection.selectedBlock)
			binfo = AC.GetBlockInfo(bid)
			light = wrld:GetBlockLight(selection.selectedBlock)
			skyLight = wrld:GetBlockSkyLight(selection.selectedBlock)
			doc:GetElementById('dbg-select-pos').inner_rml = tostring(selection.selectedBlock)
			doc:GetElementById('dbg-select-bid').inner_rml = string.format("%d:%d", bid.id, bid.state)
			doc:GetElementById('dbg-select-name').inner_rml = string.format("%s:%s", binfo.blockstate, binfo.variant)
			doc:GetElementById('dbg-select-light').inner_rml = string.format("%d:%d", light, skyLight)
		else
			doc:GetElementById('dbg-select-pos').inner_rml = ""
			doc:GetElementById('dbg-select-bid').inner_rml = ""
			doc:GetElementById('dbg-select-name').inner_rml = ""
			doc:GetElementById('dbg-select-light').inner_rml = ""
		end

		doc:GetElementById('dbg-sections-loaded').inner_rml = AC.GetDebugValue(0)
		doc:GetElementById('dbg-sections-renderer').inner_rml = AC.GetDebugValue(1)
		doc:GetElementById('dbg-sections-ready').inner_rml = AC.GetDebugValue(2)
		doc:GetElementById('dbg-sections-culled').inner_rml = AC.GetDebugValue(0) - AC.GetDebugValue(5)
		doc:GetElementById('dbg-rendered-faces').inner_rml = AC.GetDebugValue(4)

		local player = AC.GetGameState():GetPlayerStatus()
		local playerHp = string.format("%.0f", player.health)
		doc:GetElementById('status-hp').inner_rml = playerHp
		doc:GetElementById('status-hp-bar'):SetAttribute("value", playerHp)
	end


	local uiInit = optionsListenersAdded == nil
	if uiInit then
	AC.Settings.Load()
	end

	for i,v in pairs(options) do
		local input = uiDoc:GetElementById(i)
		local span = input.next_sibling

		if uiInit then
			span:AddEventListener("click", OptionsDefaultHandler, true)

			if type(v) == "number" then
				local val = AC.Settings.ReadDouble(i, v)
				input:SetAttribute("value", tostring(val))
			elseif type(v) == "boolean" then
				local val = AC.Settings.ReadBool(i, v)
				if val then
					input:SetAttribute("checked", "")
				else
					input:RemoveAttribute("checked")
				end
			end
		end

		if type(v) == "number" then
			local val = input:GetAttribute("value")
			if v == math.floor(v) and i ~= "resolutionScale" then
				span.inner_rml = string.format("%d (%d)", math.floor(val), v)
				if i == "targetFps" and val == 301 then
					span.inner_rml = string.format("∞ (%d)",  v)
				end
			else
				span.inner_rml = string.format("%.2f (%.2f)", val, v)
			end
		elseif type(v) == "boolean" then
			if v then
				span.inner_rml = "(on)"
			else
				span.inner_rml = "(off)"
			end
		end
	end

	if uiInit == true then
		optionsListenersAdded = true
		AC.SettingsUpdate()
	end
end
