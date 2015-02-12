
window = {
	pos_x = -1,
	pos_y = -1,
	width = -1,
	height = -1,
}


extention = {
	skip_error_msg = false,
--	shown_hangul_msg = false,
	fixed_ime = false,
	natural_chat = false,
	vdt_value = -1,
	sakray = false,
}

-- 未使用
-- 必要ない場合はds_format.luaで定義されるfunctionのreturn ***を削除orコメントアウト
-- もしくは返値をnilにする

chatlog = {
	normal = false,
	party = false,
	guild = false,
	whisper = false,
	broadcast = false,
	local_broadcast = false,
	talkie = false,
}


shortcut = {
	save_extended = false,
}

-- いずれも***_size>0以上で有効

font = {
	ansi_size = 0,
	ansi_weight = 400,
	ansi_name = "Arial",
	ansi_antialias = false,
	local_size = 0,
	local_weight = 400,
	local_name = "MS UI Gothic",
	local_antialias = false,
	local_charset = -1,
}


filessystem = {
	support_unicode = false,
	ask_loading_grf = false,
}


command = {
	cmd_battlemode = false,
	cmd_noshift = false,
	cmd_noctrl = false,
	cmd_window = false,
	cmd_skillfail = true,
	cmd_notrade = false,
	cmd_aura = true,
}

packet = {
	dump = false,
	message_extention = false,
}

