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
	if AC.GetGameState() and AC.GetGameState():GetPlayer() then
		local player = AC.GetGameState():GetPlayer()
		player.pos.x = player.pos.x + deltaTime * 0.5
	end
end

AC.RegisterPlugin(plugin)
plugin = nil