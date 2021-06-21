local options = {
	brightness = 0.2,
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

function UpdateUi()
	local doc = {}
	local uiDoc = {}
	for i,d in ipairs(rmlui.contexts["default"].documents) do
		if d.title == "Playing" then
			doc = d
		elseif d.title == "Options" then
			uiDoc = d
		end
    end

	if AC.GetGameState() and AC.GetGameState():GetPlayer() and AC.GetGameState():GetTimeStatus().worldAge > 0 then
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
