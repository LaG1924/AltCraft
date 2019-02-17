local plugin = {
	name = 'altcraft',
	displayName = "AltCraft Core Plugin",
	onLoad = nil,
	onUnload = nil,
	onChangeState = nil,
}

function plugin.onLoad ()
	print("Loaded AltCraft plugin!")
end

function plugin.onChangeState (newState)
	AC:LogWarning("New state: "..newState)
end

AC:RegisterPlugin(plugin)