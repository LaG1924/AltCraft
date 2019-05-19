local plugin = {
	name = 'altcraft',
	displayName = "AltCraft Core Plugin",
	onLoad = nil,
	onUnload = nil,
	onChangeState = nil,
	onTick = nil,
}

function plugin.onLoad ()
	print("Loaded AltCraft plugin!")
end

function plugin.onChangeState (newState)
	AC.LogWarning("New state: "..newState)
end

function plugin.onUnload ()
	AC.LogInfo("AC Core unloaded")
end

function plugin.onTick (deltaTime)
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

AC.RegisterPlugin(plugin)
plugin = nil