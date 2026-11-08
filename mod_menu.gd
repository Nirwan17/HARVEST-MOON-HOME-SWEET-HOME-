extends CanvasLayer
# Mod Menu + Fusion Helper + Daftar Deck + Tab Fusion + Kode Save + Campaign tanpa reset + Efek kilat/api untuk Yu-Gi-Oh Permitted Memories v1.116 (offline / single player)
# Taruh di: assets/_scripts/mod_menu.gd  (dipanggil lewat sound_control.tscn)

const PANEL_SCALE = 1.2
const PAGE_SIZE = 25

# Posisi kartu tangan saat duel (rect_position & rotasi). Urutan = card_0 .. card_4
const HAND_FLAT = [Vector2(72, 250), Vector2(256, 250), Vector2(440, 250), Vector2(624, 250), Vector2(808, 250)]
const HAND_FLAT_ROT = [0, 0, 0, 0, 0]
const HAND_CLASSIC = [Vector2(150, 280), Vector2(295, 258), Vector2(440, 250), Vector2(585, 258), Vector2(730, 280)]
const HAND_CLASSIC_ROT = [-11, -6, 0, 6, 11]

# Lencana angka fusion saat duel (rangkaian terkuat = warna pertama)
const FUSION_COLORS = [Color("e53935"), Color("fb8c00"), Color("43a047"), Color("1e88e5")]
const FUSION_COLOR_NAMES = ["Merah", "Oranye", "Hijau", "Biru"]

# Tampilan daftar deck
const ROW_H = 52
const LIST_W = 1250
const LIST_POOL = 14
const DECK_LIMIT = 100

# data pemain yang dikosongkan layar Game Over (dicadangkan lalu dikembalikan)
const SNAP_KEYS = ["player_name", "player_deck", "player_trunk", "player_starchips", "password_bought_cards", "recorded_duels", "last_reward_cards", "list_of_player_decks", "active_deck_name", "registered_freeduel_speed", "recorded_campaign_defeats", "recorded_dialogs", "game_loaded", "scene_to_return_after_duel", "going_to_duel", "last_duel_result", "tournament_last_progression_saved", "tournament_competitors_saved", "tournament_last_duelist_saved"]
const DIALOGIC_FLAGS = ["act2_tea", "act2_tristan", "act2_bakura", "act2_duke", "act2_grandpa"]
# kolom baris daftar: no, ikon atribut, nama, ATK/DEF, level, tipe, jenis, jumlah, tombol aksi
const COL_X = [8, 84, 124, 512, 710, 788, 956, 1074, 1156]
const COL_W = [70, 32, 380, 190, 70, 160, 110, 60, 66]
const BADGE_PHASES = ["draw_phase", "main_phase", "looking_at_hand", "selecting_field_slot", "selecting_combat_target", "choosing_combat_options", "card_options", "activating_equip_from_field"]

var _panel
var _toggle
var _status
var _god_btn
var _god_timer
var _god_on = false

var _fh_panel
var _fh_out
var _fh_chain_btn
var _fh_field_btn
var _fh_more_btn
var _fh_chain = true
var _fh_field = true
var _deck_summary = ""
var _deck_lines = []
var _deck_shown = 0
var _fusion_inst = null

var _pv_root
var _pv_panel
var _pv_card = null
var _pv_title
var _pv_attr_icon
var _pv_type_icon
var _pv_meta
var _pv_stats
var _pv_own
var _pv_desc
var _pv_fusion
var _pv_timer
var _pv_auto_btn
var _auto_preview = true
var _last_hl = 0

var _hand_flat = true
var _hand_btn

var _fn_on = true
var _lg_on = true
var _fn_btn
var _lg_btn
var _badge_layer
var _badge_font = null
var _badge_timer
var _bg = []
var _bsig = ""
var _badges_shown = false
var _legend_panel
var _legend_box
var _legend_has = false

var _btn_pos = Vector2(0, 0)
var _btn_saved = false
var _tg_down = false
var _tg_dragging = false
var _tg_press_ms = 0
var _tg_grab = Vector2(0, 0)
var _list_btn
var _list_mode = true
var _ls_mode_btn
var _ls_root
var _ls_count
var _ls_lists = {}
var _ls_sort_btns = {}
var _ls_no_sort = false
var _ls_duel_btn
var _ls_timer
var _list_font = null
var _new_font = null
var _stamp_font = null
var _ls_new_sort = false
var _ls_page = "trunk"
var _ls_tab_btns = {}
var _duel_all = false
var _duel_btn
var _imp_armed_ms = -100000

var _small_font = null
var _ls_heads = []
var _fx_heads = []
var _fx_ctrl = []
var _fx_stat_btn
var _fx_src_btn
var _fx_val_lbl
var _fx_info
var _fx_job = null
var _fx_src = "deck"
var _fx_stat = "atk"
var _fx_val = 2200
var _fx_store = {"sig": "", "res": [], "done": false}
var _fx_show_nf = true
var _fx_keep_scroll = false
var _bulk_armed_ms = -100000
var _fx_bulk_btn
var _fx_len = 2
var _fx_len_btn

var _snap = {}
var _go_root
var _go_panel
var _go_title
var _go_text
var _go_btn1
var _go_btn2
var _go_kind = ""
var _go_last_dlg = ""
var _last_story_opp = ""
var _go_repeat = 0
var _ef_on = true
var _ef_btn
var _ef_flash
var _ef_flash_a = 0.0
var _ef_flash_col = Color(1, 1, 1, 1)
var _ef_items = []
var _ef_state = {}
var _ef_reward_done = false
var _fx_head_best
var _fx_nf_btn

var _pv_id = ""
var _pv_fusion_title
var _pv_fusion_box
var _pv_act_row
var _pv_add_btn
var _pv_rem_btn
var _ct = null
var _fx_cache = {}
var _fx_sig = ""


func _ready():
	# jaga-jaga kalau scene ini ke-instance lebih dari sekali
	if get_tree().root.has_meta("mod_menu_loaded"):
		queue_free()
		return
	get_tree().root.set_meta("mod_menu_loaded", true)
	layer = 128
	_badge_layer = Control.new()
	_badge_layer.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(_badge_layer)
	var fd = load("res://_resources/fonts/BankGothic Md BT.ttf")
	if fd != null:
		_badge_font = DynamicFont.new()
		_badge_font.font_data = fd
		_badge_font.size = 22
		_badge_font.outline_size = 2
		_badge_font.outline_color = Color(0, 0, 0, 1)
	_build_legend()
	_cfg_load()
	_build_effects()
	_build_list()
	_build_preview()
	_build_gameover()
	_build_ui()
	_build_fusion_panel()
	_god_timer = Timer.new()
	_god_timer.wait_time = 0.25
	_god_timer.one_shot = false
	add_child(_god_timer)
	_god_timer.connect("timeout", self, "_god_tick")
	_god_timer.start()
	_pv_timer = Timer.new()
	_pv_timer.wait_time = 0.2
	_pv_timer.one_shot = false
	add_child(_pv_timer)
	_pv_timer.connect("timeout", self, "_pv_tick")
	_pv_timer.start()
	_badge_timer = Timer.new()
	_badge_timer.wait_time = 0.3
	_badge_timer.one_shot = false
	add_child(_badge_timer)
	_badge_timer.connect("timeout", self, "_badge_tick")
	_badge_timer.start()
	_ls_timer = Timer.new()
	_ls_timer.wait_time = 0.25
	_ls_timer.one_shot = false
	add_child(_ls_timer)
	_ls_timer.connect("timeout", self, "_ls_tick")
	_ls_timer.start()
	set_process(true)
	# pasang tata letak tangan lurus begitu node player_logic (duel) muncul
	get_tree().connect("node_added", self, "_on_node_added")


func _build_ui():
	var vp = get_viewport().get_visible_rect().size

	_toggle = Button.new()
	_toggle.text = "MOD"
	_toggle.rect_min_size = Vector2(56, 32)
	_toggle.rect_size = Vector2(56, 32)
	_toggle.focus_mode = Control.FOCUS_NONE
	_toggle.modulate = Color(1, 1, 1, 0.45)
	var bx = vp.x - 62
	var by = 4.0
	if _btn_saved:
		bx = clamp(_btn_pos.x, 0, vp.x - 56)
		by = clamp(_btn_pos.y, 0, vp.y - 32)
	_toggle.rect_position = Vector2(bx, by)
	_toggle.connect("gui_input", self, "_on_toggle_input")
	add_child(_toggle)

	_list_btn = Button.new()
	_list_btn.text = "LIST"
	_list_btn.rect_min_size = Vector2(56, 32)
	_list_btn.rect_size = Vector2(56, 32)
	_list_btn.focus_mode = Control.FOCUS_NONE
	_list_btn.modulate = Color(1, 1, 1, 0.6)
	_list_btn.visible = false
	_list_btn.rect_position = Vector2(bx - 60, by)
	_list_btn.connect("pressed", self, "_on_ls_mode")
	add_child(_list_btn)

	_panel = PanelContainer.new()
	_panel.rect_scale = Vector2(PANEL_SCALE, PANEL_SCALE)
	_panel.rect_position = Vector2(vp.x - 340 * PANEL_SCALE - 90, 20)
	_panel.visible = false
	add_child(_panel)

	var scroll = ScrollContainer.new()
	scroll.rect_min_size = Vector2(340, 420)
	scroll.scroll_horizontal_enabled = false
	_panel.add_child(scroll)

	var box = VBoxContainer.new()
	box.rect_min_size = Vector2(320, 0)
	box.add_constant_override("separation", 6)
	scroll.add_child(box)

	_add_label(box, "MOD MENU - Permitted Memories")
	_status = _add_label(box, "Siap.")
	_add_label(box, "--- Fusion Helper ---")
	_add_btn(box, "Buka / tutup Fusion Helper", "_on_fh_toggle")
	_add_label(box, "--- Tampilan ---")
	_pv_auto_btn = _add_btn(box, "Auto-pratinjau kartu (deck): ON", "_on_pv_auto")
	_add_btn(box, "Pratinjau kartu terpilih", "_on_pv_manual")
	_hand_btn = _add_btn(box, "Tangan duel: LURUS", "_on_hand_toggle")
	_fn_btn = _add_btn(box, "Angka fusion duel: ON", "_on_fn_toggle")
	_lg_btn = _add_btn(box, "Legenda fusion: ON", "_on_lg_toggle")
	_ls_mode_btn = _add_btn(box, "Tampilan deck: " + ("DAFTAR" if _list_mode else "ASLI"), "_on_ls_mode")
	_duel_btn = _add_btn(box, "Deck saat duel: " + ("SEMUA" if _duel_all else "40 acak"), "_on_duel_mode")
	_ef_btn = _add_btn(box, "Efek kilat/api: " + ("ON" if _ef_on else "OFF"), "_on_ef_toggle")
	_add_label(box, "--- Save data ---")
	_add_btn(box, "Salin kode save", "_on_save_export")
	_add_btn(box, "Impor dari clipboard", "_on_save_import")
	_add_label(box, "Tahan tombol MOD lalu seret untuk memindahkannya.")
	_add_label(box, "--- Starchips & Kartu ---")
	_add_btn(box, "+10.000 Starchips", "_on_stars_10k")
	_add_btn(box, "Starchips = 99.999", "_on_stars_max")
	_add_btn(box, "Unlock semua kartu (+1 tiap kartu)", "_on_unlock_all")
	_add_btn(box, "Semua kartu jadi 3 kopi", "_on_all_three")
	_add_btn(box, "Simpan game", "_on_save")
	_add_label(box, "--- Duel (hanya saat duel) ---")
	_add_btn(box, "Menang instan", "_on_win")
	_add_btn(box, "LP kamu penuh (9999)", "_on_heal")
	_god_btn = _add_btn(box, "Kunci LP kamu 9999: MATI", "_on_god")
	_add_btn(box, "Tutup menu", "_on_toggle")


func _add_label(box, text):
	var l = Label.new()
	l.text = text
	l.autowrap = true
	l.rect_min_size = Vector2(320, 0)
	box.add_child(l)
	return l


func _add_btn(box, text, method):
	var b = Button.new()
	b.text = text
	b.rect_min_size = Vector2(0, 38)
	b.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	b.connect("pressed", self, method)
	box.add_child(b)
	return b


func _say(msg):
	_status.text = str(msg)


func _on_toggle():
	_panel.visible = not _panel.visible


# ---------- Starchips & kartu ----------

func _player_data():
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null:
		_say("PlayerData tidak ditemukan.")
		return null
	if pd.game_loaded != true:
		_say("Masuk dulu: pilih Load Game atau New Game di menu utama.")
		return null
	return pd


func _on_stars_10k():
	var pd = _player_data()
	if pd == null:
		return
	pd.player_starchips += 10000
	_finish("Starchips sekarang " + str(pd.player_starchips))


func _on_stars_max():
	var pd = _player_data()
	if pd == null:
		return
	pd.player_starchips = 99999
	_finish("Starchips sekarang 99999")


func _on_unlock_all():
	var pd = _player_data()
	if pd == null:
		return
	var cl = get_node_or_null("/root/CardList")
	if cl == null:
		_say("CardList tidak ditemukan.")
		return
	var n = 0
	for k in cl.card_list.keys():
		if not str(k).is_valid_integer():
			continue
		var id = str(k).pad_zeros(5)
		if pd.player_trunk.has(id):
			pd.player_trunk[id] += 1
		else:
			pd.player_trunk[id] = 1
		n += 1
	_finish("+1 kopi untuk " + str(n) + " kartu")


func _on_all_three():
	var pd = _player_data()
	if pd == null:
		return
	var cl = get_node_or_null("/root/CardList")
	if cl == null:
		_say("CardList tidak ditemukan.")
		return
	var n = 0
	for k in cl.card_list.keys():
		if not str(k).is_valid_integer():
			continue
		var id = str(k).pad_zeros(5)
		if not pd.player_trunk.has(id) or pd.player_trunk[id] < 3:
			pd.player_trunk[id] = 3
		n += 1
	_finish("Semua " + str(n) + " kartu minimal 3 kopi")


func _on_save():
	if _player_data() == null:
		return
	_say("Simpan: " + ("berhasil" if _save() else "GAGAL"))


func _save():
	var script = load("res://_scripts/save_load_logic.gd")
	if script == null:
		return false
	var sl = script.new()
	var r = sl.save_game()
	sl.free()
	return r == "sucess"


func _finish(msg):
	var saved = _save()
	var note = ""
	if _refresh_scene():
		note = " (layar dimuat ulang)"
	else:
		note = " (buka ulang layar Deck Building / Card Shop untuk melihat)"
	_say(msg + " | " + ("tersimpan" if saved else "GAGAL simpan") + note)


func _refresh_scene():
	var scn = get_tree().current_scene
	if scn == null:
		return false
	var f = str(scn.filename)
	if f.ends_with("deck_building.tscn") or f.ends_with("card_shop.tscn"):
		get_tree().reload_current_scene()
		return true
	return false


# ---------- Duel ----------

func _duel_scene():
	var scn = get_tree().current_scene
	if scn == null or not scn.has_node("game_logic"):
		_say("Ini bukan layar duel.")
		return null
	return scn


func _on_win():
	var scn = _duel_scene()
	if scn == null:
		return
	scn.get_node("game_logic").check_for_game_end("DEBUG_END_DUEL")
	_say("Menang instan.")


func _on_heal():
	var scn = _duel_scene()
	if scn == null:
		return
	scn.get_node("game_logic").change_lifepoints("player", 9999, true)
	_say("LP kamu diisi penuh.")


func _on_god():
	_god_on = not _god_on
	_god_btn.text = "Kunci LP kamu 9999: " + ("HIDUP" if _god_on else "MATI")


func _god_tick():
	if not _god_on:
		return
	var scn = get_tree().current_scene
	if scn == null:
		return
	var path = "user_interface/top_info_box/player_info/lifepoints"
	if not scn.has_node(path):
		return
	var lbl = scn.get_node(path)
	var v = int(lbl.text)
	if v > 0 and v < 9999:
		lbl.text = "9999"


# ---------- Fusion Helper: tampilan ----------

func _build_fusion_panel():
	_fh_panel = PanelContainer.new()
	_fh_panel.rect_scale = Vector2(PANEL_SCALE, PANEL_SCALE)
	_fh_panel.rect_position = Vector2(16, 16)
	_fh_panel.visible = false
	add_child(_fh_panel)

	var fbox = VBoxContainer.new()
	fbox.rect_min_size = Vector2(400, 0)
	fbox.add_constant_override("separation", 6)
	_fh_panel.add_child(fbox)

	_add_label(fbox, "FUSION HELPER")

	var row1 = HBoxContainer.new()
	fbox.add_child(row1)
	_add_btn(row1, "Tangan", "_on_fh_hand")
	_add_btn(row1, "Deck", "_on_fh_deck")
	_add_btn(row1, "Tutup", "_on_fh_toggle")

	var row2 = HBoxContainer.new()
	fbox.add_child(row2)
	_fh_chain_btn = _add_btn(row2, "Berantai: ON", "_on_fh_chain")
	_fh_field_btn = _add_btn(row2, "Lapangan: ON", "_on_fh_field")

	var sc = ScrollContainer.new()
	sc.rect_min_size = Vector2(400, 300)
	sc.scroll_horizontal_enabled = false
	fbox.add_child(sc)

	_fh_out = Label.new()
	_fh_out.autowrap = true
	_fh_out.rect_min_size = Vector2(380, 0)
	_fh_out.text = "Tangan = hitung fusion dari kartu di tangan (saat duel).\nDeck = analisis fusion dari semua kartu di deck-mu (di mana saja)."
	sc.add_child(_fh_out)

	_fh_more_btn = _add_btn(fbox, "Tampilkan lebih banyak", "_on_fh_more")
	_fh_more_btn.visible = false


func _on_fh_toggle():
	_fh_panel.visible = not _fh_panel.visible


func _on_fh_chain():
	_fh_chain = not _fh_chain
	_fh_chain_btn.text = "Berantai: " + ("ON" if _fh_chain else "OFF")


func _on_fh_field():
	_fh_field = not _fh_field
	_fh_field_btn.text = "Lapangan: " + ("ON" if _fh_field else "OFF")


func _fh_say(text):
	_fh_out.text = str(text)
	_fh_more_btn.visible = false


# ---------- Fusion Helper: data kartu ----------

func _info(id):
	var cl = get_node_or_null("/root/CardList")
	if cl == null:
		return null
	if not cl.card_list.has(id):
		return null
	return cl.card_list[id]


func _name(id):
	var inf = _info(id)
	if inf == null:
		return "?" + str(id)
	return str(inf.get("card_name", id))


func _is_spelltrap(id):
	var inf = _info(id)
	if inf == null:
		return false
	var a = str(inf.get("attribute", ""))
	return a == "spell" or a == "trap"


func _atk(id):
	var inf = _info(id)
	if inf == null:
		return 0
	return int(inf.get("atk", 0))


func _def(id):
	var inf = _info(id)
	if inf == null:
		return 0
	return int(inf.get("def", 0))


func _stat_text(id, ab, db):
	if _is_spelltrap(id):
		return ""
	return "[ATK " + str(_atk(id) + ab) + " / DEF " + str(_def(id) + db) + "]"


func _signed(n):
	if n >= 0:
		return "+" + str(n)
	return str(n)


func _stat_label(code):
	if code == "atk_up":
		return "ATK"
	if code == "def_up":
		return "DEF"
	return "ATK/DEF"


# Pakai node fusion asli milik game kalau sedang duel (aturan sama persis),
# kalau tidak, buat satu salinan dari script fusions.gd.
func _fusion_node(scn):
	if scn != null and scn.has_node("game_logic"):
		var gl = scn.get_node("game_logic")
		var fl = gl.get("fusion_list")
		if fl != null:
			return fl
	if _fusion_inst == null:
		var s = load("res://_scripts/fusions.gd")
		if s != null:
			_fusion_inst = s.new()
	return _fusion_inst


# Hasil: null = tidak ada fusion.
# [id_hasil, null]  = fusion monster biasa
# [id_monster, [stat, jumlah]] = equip berhasil
func _fuse(fi, a, b):
	var r = fi.check_for_fusion(a, b)
	if typeof(r) != TYPE_ARRAY or r.size() < 2:
		return null
	var flag = r[1]
	if typeof(flag) == TYPE_BOOL and flag == true:
		return [str(r[0]), null]
	if typeof(flag) == TYPE_ARRAY and flag.size() >= 2 and flag[1] != 0:
		return [str(r[0]), flag]
	return null


func _step_label(a, b, nid, flag):
	if flag == null:
		return _name(a) + " + " + _name(b) + " -> " + _name(nid)
	var amt = int(flag[1])
	return _name(a) + " + " + _name(b) + " -> " + _name(nid) + " " + _signed(amt) + " " + _stat_label(str(flag[0]))


# Cari semua urutan fusion dari kartu 'cur' dengan kartu-kartu di 'hand'.
func _dfs(fi, cur, hand, used, steps, pkey, out, use_chain, srcs):
	for j in range(hand.size()):
		if used & (1 << j):
			continue
		var res = _fuse(fi, cur["id"], hand[j])
		if res == null:
			continue
		var nid = res[0]
		var flag = res[1]
		var ab = cur["ab"]
		var db = cur["db"]
		if flag == null:
			if nid != cur["id"]:
				ab = 0
				db = 0
		else:
			var amt = int(flag[1])
			var code = str(flag[0])
			if code == "atk_up":
				ab += amt
			elif code == "def_up":
				db += amt
			else:
				ab += amt
				db += amt
		var key = ""
		if steps.size() == 0:
			var pair = [cur["id"], hand[j]]
			pair.sort()
			key = pair[0] + "+" + pair[1] + ">" + nid + "/" + str(flag)
		else:
			key = pkey + "|" + hand[j] + ">" + nid + "/" + str(flag)
		var nsteps = steps.duplicate()
		nsteps.append(_step_label(cur["id"], hand[j], nid, flag))
		var nsrcs = srcs.duplicate()
		nsrcs.append("h" + str(j))
		out.append({"steps": nsteps, "id": nid, "ab": ab, "db": db, "key": key, "srcs": nsrcs})
		if use_chain:
			var nxt = {"id": nid, "ab": ab, "db": db}
			_dfs(fi, nxt, hand, used | (1 << j), nsteps, key, out, use_chain, nsrcs)


func _cmp_score(a, b):
	return a["score"] > b["score"]


func _rank(arr):
	var seen = {}
	var uniq = []
	for e in arr:
		if seen.has(e["key"]):
			continue
		seen[e["key"]] = true
		e["score"] = _atk(e["id"]) + e["ab"]
		uniq.append(e)
	uniq.sort_custom(self, "_cmp_score")
	return uniq


func _fmt_entry(e):
	var s = ""
	var n = e["steps"].size()
	for k in range(n):
		if k > 0:
			s += "; "
		s += e["steps"][k]
	var st = _stat_text(e["id"], e["ab"], e["db"])
	if n > 1:
		s += "  => " + _name(e["id"])
	if st != "":
		s += " " + st
	return s


func _append_section(text, title, arr, limit):
	if arr.size() == 0:
		return text
	text += "\n== " + title + " (" + str(arr.size()) + ") ==\n"
	var shown = min(arr.size(), limit)
	for i in range(shown):
		text += "- " + _fmt_entry(arr[i]) + "\n"
	if arr.size() > shown:
		text += "(+" + str(arr.size() - shown) + " lainnya)\n"
	return text


# ---------- Fusion Helper: tangan (saat duel) ----------

func _on_fh_hand():
	var scn = get_tree().current_scene
	if scn == null or not scn.has_node("game_logic/player_logic"):
		_fh_say("Ini bukan layar duel. Masuk ke duel dulu, atau pakai tombol Deck.")
		return
	var fi = _fusion_node(scn)
	if fi == null:
		_fh_say("Gagal memuat logika fusion.")
		return
	var gl = scn.get_node("game_logic")
	var hand = []
	for id in gl.get_node("player_logic").player_hand:
		hand.append(str(id))
	if hand.size() == 0:
		_fh_say("Tangan kosong.")
		return

	# Daftar monster milik kita di lapangan
	var fields = []
	if _fh_field:
		for i in range(5):
			var n = scn.get_node_or_null("duel_field/player_side_zones/monster_" + str(i))
			if n != null and n.is_visible():
				var fl = n.this_card_flags
				fields.append({"id": str(n.this_card_id), "ab": int(fl.get("atk_up", 0)), "db": int(fl.get("def_up", 0)), "slot": i})

	# Beberapa equip khusus menghitung kartu spell/trap di sisi pemain
	var old_phase = gl.GAME_PHASE
	gl.GAME_PHASE = "checking_for_fusions"

	var pairs = []
	var chains = []
	for i in range(hand.size()):
		var out = []
		var cur = {"id": hand[i], "ab": 0, "db": 0}
		_dfs(fi, cur, hand, 1 << i, [], "", out, _fh_chain, ["h" + str(i)])
		for e in out:
			if e["steps"].size() == 1:
				pairs.append(e)
			else:
				chains.append(e)

	var fieldres = []
	for f in fields:
		var out2 = []
		var cur2 = {"id": f["id"], "ab": f["ab"], "db": f["db"]}
		_dfs(fi, cur2, hand, 0, [], "", out2, _fh_chain, ["f" + str(f["slot"])])
		for e2 in out2:
			e2["steps"][0] = "[Lapangan " + _name(f["id"]) + ", slot " + str(f["slot"] + 1) + "] " + e2["steps"][0]
			e2["key"] = "F" + str(f["slot"]) + "|" + e2["key"]
			fieldres.append(e2)

	gl.GAME_PHASE = old_phase

	var names = ""
	for k in range(hand.size()):
		if k > 0:
			names += ", "
		names += _name(hand[k])
	var text = "TANGAN (" + str(hand.size()) + "): " + names + "\n"
	text += "Urutan kartu = urutan kamu memilih kartu untuk fusi.\n"
	text = _append_section(text, "Fusion 2 kartu", _rank(pairs), 12)
	if _fh_chain:
		text = _append_section(text, "Berantai terbaik", _rank(chains), 8)
	if _fh_field:
		text = _append_section(text, "Dengan monster di lapangan", _rank(fieldres), 8)
	if pairs.size() == 0 and chains.size() == 0 and fieldres.size() == 0:
		text += "\nTidak ada fusion yang bisa dibuat dari kartu sekarang."
	_fh_say(text)


# ---------- Fusion Helper: analisis deck ----------

func _on_fh_deck():
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null or pd.player_deck.size() == 0:
		_fh_say("Deck belum ada. Masuk dulu ke game (Load Game / New Game).")
		return
	var scn = get_tree().current_scene
	var fi = _fusion_node(scn)
	if fi == null:
		_fh_say("Gagal memuat logika fusion.")
		return

	var counts = {}
	var uniq = []
	for id in pd.player_deck:
		var sid = str(id)
		if counts.has(sid):
			counts[sid] += 1
		else:
			counts[sid] = 1
			uniq.append(sid)

	var seen = {}
	var mons = []
	var equips = []
	for i in range(uniq.size()):
		for j in range(i, uniq.size()):
			if i == j and counts[uniq[i]] < 2:
				continue
			for order in range(2):
				if order == 1 and i == j:
					continue
				var a = uniq[i]
				var b = uniq[j]
				if order == 1:
					a = uniq[j]
					b = uniq[i]
				var res = _fuse(fi, a, b)
				if res == null:
					continue
				var nid = res[0]
				var flag = res[1]
				var lo = a
				var hi = b
				if a > b:
					lo = b
					hi = a
				var key = lo + "+" + hi + ">" + nid + "/" + str(flag)
				if seen.has(key):
					continue
				seen[key] = true
				var text = _step_label(a, b, nid, flag)
				if flag == null:
					mons.append({"score": _atk(nid), "text": text + " " + _stat_text(nid, 0, 0)})
				else:
					equips.append({"score": int(flag[1]), "text": text})

	mons.sort_custom(self, "_cmp_score")
	equips.sort_custom(self, "_cmp_score")

	_deck_lines = []
	for m in mons:
		_deck_lines.append(m["text"])
	if equips.size() > 0:
		_deck_lines.append("-- Equip / boost --")
		for q in equips:
			_deck_lines.append(q["text"])

	_deck_summary = "DECK: " + str(pd.player_deck.size()) + " kartu (" + str(uniq.size()) + " unik)\n"
	_deck_summary += "Fusion monster: " + str(mons.size()) + " | Equip/boost: " + str(equips.size()) + "\n"
	_deck_summary += "Catatan: equip khusus yang bergantung pada isi lapangan hanya terhitung saat duel.\n"
	_deck_shown = 0
	if _deck_lines.size() == 0:
		_fh_say(_deck_summary + "\nTidak ada kombinasi fusion di deck ini.")
		return
	_deck_show_more()


func _on_fh_more():
	_deck_show_more()


func _deck_show_more():
	_deck_shown = min(_deck_shown + PAGE_SIZE, _deck_lines.size())
	var text = _deck_summary + "\n"
	for i in range(_deck_shown):
		text += "- " + _deck_lines[i] + "\n"
	_fh_out.text = text
	_fh_more_btn.visible = _deck_shown < _deck_lines.size()
	if _fh_more_btn.visible:
		_fh_more_btn.text = "Tampilkan lebih banyak (" + str(_deck_lines.size() - _deck_shown) + " lagi)"


# ---------- Pratinjau kartu besar ----------

func _build_preview():
	var vp = get_viewport().get_visible_rect().size
	_pv_root = Control.new()
	_pv_root.rect_min_size = vp
	_pv_root.rect_size = vp
	_pv_root.mouse_filter = Control.MOUSE_FILTER_STOP
	_pv_root.visible = false
	_pv_root.connect("gui_input", self, "_on_pv_input")
	add_child(_pv_root)

	var dim = ColorRect.new()
	dim.color = Color(0, 0, 0, 0.82)
	dim.rect_size = vp
	dim.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_pv_root.add_child(dim)

	_pv_panel = PanelContainer.new()
	_pv_panel.rect_scale = Vector2(1.25, 1.25)
	_pv_panel.rect_position = Vector2(440, 50)
	_pv_panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_pv_root.add_child(_pv_panel)

	var box = VBoxContainer.new()
	box.rect_min_size = Vector2(600, 0)
	box.add_constant_override("separation", 8)
	_pv_panel.add_child(box)

	_pv_title = _add_label(box, "")
	_pv_title.rect_min_size = Vector2(600, 0)

	var row = HBoxContainer.new()
	row.add_constant_override("separation", 10)
	box.add_child(row)
	_pv_attr_icon = TextureRect.new()
	_pv_attr_icon.expand = true
	_pv_attr_icon.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	_pv_attr_icon.rect_min_size = Vector2(44, 44)
	row.add_child(_pv_attr_icon)
	_pv_type_icon = TextureRect.new()
	_pv_type_icon.expand = true
	_pv_type_icon.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	_pv_type_icon.rect_min_size = Vector2(44, 44)
	row.add_child(_pv_type_icon)
	_pv_meta = Label.new()
	_pv_meta.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	row.add_child(_pv_meta)

	_pv_stats = _add_label(box, "")
	_pv_stats.rect_min_size = Vector2(600, 0)
	_pv_own = _add_label(box, "")
	_pv_own.rect_min_size = Vector2(600, 0)

	_pv_act_row = HBoxContainer.new()
	_pv_act_row.add_constant_override("separation", 8)
	box.add_child(_pv_act_row)
	_pv_add_btn = _add_btn(_pv_act_row, "+ Tambah ke deck", "_on_pv_add")
	_pv_rem_btn = _add_btn(_pv_act_row, "- Keluarkan dari deck", "_on_pv_rem")

	var sc = ScrollContainer.new()
	sc.rect_min_size = Vector2(600, 110)
	sc.scroll_horizontal_enabled = false
	box.add_child(sc)
	_pv_desc = Label.new()
	_pv_desc.autowrap = true
	_pv_desc.rect_min_size = Vector2(580, 0)
	sc.add_child(_pv_desc)

	_pv_fusion_title = _add_label(box, "Fusion terkuat (dari kartu yang kamu miliki):")
	_pv_fusion_title.rect_min_size = Vector2(600, 0)
	_pv_fusion_box = VBoxContainer.new()
	_pv_fusion_box.add_constant_override("separation", 6)
	box.add_child(_pv_fusion_box)

	_add_btn(box, "Tutup", "_pv_close")


func _ensure_pv_card():
	if _pv_card != null:
		return
	var cs = load("res://_scenes/card_visual_only.tscn")
	if cs == null:
		return
	_pv_card = cs.instance()
	_pv_root.add_child(_pv_card)
	_pv_card.rect_position = Vector2(70, 120)
	_pv_card.rect_scale = Vector2(1.05, 1.05)
	_pv_card.mouse_filter = Control.MOUSE_FILTER_IGNORE


func _pv_close():
	_pv_root.visible = false


func _on_pv_input(event):
	var pos = null
	if event is InputEventMouseButton and event.pressed:
		pos = event.position
	elif event is InputEventScreenTouch and event.pressed:
		pos = event.position
	if pos == null:
		return
	var r = Rect2(_pv_panel.rect_position, _pv_panel.rect_size * _pv_panel.rect_scale)
	if r.has_point(pos):
		return
	_pv_close()


func _on_pv_auto():
	_auto_preview = not _auto_preview
	_pv_auto_btn.text = "Auto-pratinjau kartu (deck): " + ("ON" if _auto_preview else "OFF")


func _on_pv_manual():
	var id = _current_card_id()
	if id == "":
		_say("Belum ada kartu terpilih. Tap sebuah kartu dulu.")
		return
	_panel.visible = false
	_show_preview(id)


func _info_box():
	var scn = get_tree().current_scene
	if scn == null:
		return null
	return scn.get_node_or_null("user_interface/card_info_box")


func _current_card_id():
	var ib = _info_box()
	if ib == null:
		return ""
	var c = ib.current_highlighted_card
	if c != null and is_instance_valid(c):
		return str(c.this_card_id)
	# cadangan: cocokkan nama kartu yang tampil di kotak info
	var nm = ib.get_node_or_null("card_name/card_name")
	var cl = get_node_or_null("/root/CardList")
	if nm == null or cl == null or str(nm.text) == "":
		return ""
	for k in cl.card_list.keys():
		if str(cl.card_list[k].get("card_name", "")) == str(nm.text):
			return str(k)
	return ""


func _pv_tick():
	if not _auto_preview or _pv_root.visible:
		return
	var scn = get_tree().current_scene
	if scn == null:
		return
	if not str(scn.filename).ends_with("deck_building.tscn"):
		_last_hl = 0
		return
	var ib = _info_box()
	if ib == null:
		return
	var c = ib.current_highlighted_card
	if c == null or not is_instance_valid(c):
		_last_hl = 0
		return
	var iid = c.get_instance_id()
	if iid == _last_hl:
		return
	_last_hl = iid
	_show_preview(str(c.this_card_id))


func _show_preview(id):
	var inf = _info(id)
	if inf == null:
		return
	_ensure_pv_card()
	_pv_id = id
	if _pv_card != null:
		_pv_card.update_card_information(id)

	var attr = str(inf.get("attribute", ""))
	var typ = str(inf.get("type", ""))
	_pv_title.text = str(inf.get("card_name", id)).to_upper()
	_pv_attr_icon.texture = load("res://_resources/_attributes/" + attr + ".png")
	_pv_type_icon.texture = load("res://_resources/_types/" + typ + ".png")

	var meta = "Atribut: " + attr.capitalize() + "\nTipe: " + typ.capitalize()
	if not _is_spelltrap(id):
		meta += "\nLevel: " + str(inf.get("level", 0))
	_pv_meta.text = meta

	var kind = ""
	if attr == "spell":
		kind = "Spell"
	elif attr == "trap":
		kind = "Trap"
	else:
		var eff = inf.get("effect", [])
		if typeof(eff) == TYPE_ARRAY and eff.size() > 0:
			kind = "Monster Efek"
		else:
			kind = "Monster Normal"
	if typ == "ritual":
		kind += " (Ritual)"
	var stats = "Jenis: " + kind
	if not _is_spelltrap(id):
		stats += "\nATK " + str(_atk(id)) + "   DEF " + str(_def(id))
	_pv_stats.text = stats

	var pd = get_node_or_null("/root/PlayerData")
	if pd != null:
		var owned = 0
		if pd.player_trunk.has(id):
			owned = int(pd.player_trunk[id])
		_pv_own.text = "Di koleksi: " + str(owned) + "   |   Di deck: " + str(pd.player_deck.count(id)) + " / 3"
	else:
		_pv_own.text = ""

	_pv_desc.text = _card_desc(id)

	_fill_fusion_rows(id)
	_pv_update_actions(id)
	_pv_root.visible = true


func _fusion_lines_for(id):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null or pd.player_deck.size() == 0:
		return "-"
	var scn = get_tree().current_scene
	var fi = _fusion_node(scn)
	if fi == null:
		return "-"
	var uniq = []
	for d in pd.player_deck:
		var sid = str(d)
		if not uniq.has(sid):
			uniq.append(sid)
	var mons = []
	var equips = 0
	var seen = {}
	for o in uniq:
		if o == id and pd.player_deck.count(id) < 2:
			continue
		for order in range(2):
			if order == 1 and o == id:
				continue
			var a = id
			var b = o
			if order == 1:
				a = o
				b = id
			var res = _fuse(fi, a, b)
			if res == null:
				continue
			var lo = a
			var hi = b
			if a > b:
				lo = b
				hi = a
			var key = lo + "+" + hi + ">" + res[0] + "/" + str(res[1])
			if seen.has(key):
				continue
			seen[key] = true
			if res[1] == null:
				mons.append({"score": _atk(res[0]), "text": _step_label(a, b, res[0], null) + " " + _stat_text(res[0], 0, 0)})
			else:
				equips += 1
	mons.sort_custom(self, "_cmp_score")
	var text = ""
	var shown = min(mons.size(), 8)
	for i in range(shown):
		text += "- " + mons[i]["text"] + "\n"
	if mons.size() > shown:
		text += "(+" + str(mons.size() - shown) + " fusion lainnya)\n"
	if equips > 0:
		text += "(+" + str(equips) + " kombinasi equip/boost)\n"
	if text == "":
		return "Tidak ada fusion dengan kartu di deck-mu."
	return text


# ---------- Tangan duel lurus ----------

func _on_node_added(node):
	if node.get_script() != null and node.has_method("reset_player_data_to_empty_status"):
		_take_snapshot()
		call_deferred("_after_game_over", node)
	if node.name == "player_logic" and node.has_method("player_draw_phase"):
		call_deferred("_trim_duel_deck", node)
		if not _apply_hand_layout(node):
			_say("Tangan duel: gagal mengubah posisi kartu.")


# Tabel card_references di script game bisa diubah saat game berjalan.
func _apply_hand_layout(pl):
	var refs = pl.card_references
	if typeof(refs) != TYPE_DICTIONARY:
		return false
	for i in range(5):
		var key = "card_" + str(i) + "_references"
		if not refs.has(key):
			continue
		if _hand_flat:
			refs[key]["rect_position"] = HAND_FLAT[i]
			refs[key]["rect_rotation"] = HAND_FLAT_ROT[i]
		else:
			refs[key]["rect_position"] = HAND_CLASSIC[i]
			refs[key]["rect_rotation"] = HAND_CLASSIC_ROT[i]
	if not refs.has("card_0_references"):
		return false
	var want = HAND_CLASSIC_ROT[0]
	if _hand_flat:
		want = HAND_FLAT_ROT[0]
	return refs["card_0_references"]["rect_rotation"] == want


func _on_hand_toggle():
	_hand_flat = not _hand_flat
	_hand_btn.text = "Tangan duel: " + ("LURUS" if _hand_flat else "KIPAS (asli)")
	var scn = get_tree().current_scene
	if scn == null or not scn.has_node("game_logic/player_logic"):
		_say("Pengaturan tangan disimpan, berlaku di duel berikutnya.")
		return
	var ok = _apply_hand_layout(scn.get_node("game_logic/player_logic"))
	_relayout_hand_now(scn)
	_say("Tangan duel: " + ("LURUS" if _hand_flat else "KIPAS") + (" (OK)" if ok else " (GAGAL mengubah posisi)"))


# Pindahkan kartu yang sedang diam di posisi tangan ke tata letak baru.
func _relayout_hand_now(scn):
	for i in range(5):
		var n = scn.get_node_or_null("player_hand/card_" + str(i))
		if n == null or not n.is_visible():
			continue
		var old_pos = HAND_CLASSIC[i]
		var old_rot = HAND_CLASSIC_ROT[i]
		var new_pos = HAND_FLAT[i]
		var new_rot = HAND_FLAT_ROT[i]
		if not _hand_flat:
			old_pos = HAND_FLAT[i]
			old_rot = HAND_FLAT_ROT[i]
			new_pos = HAND_CLASSIC[i]
			new_rot = HAND_CLASSIC_ROT[i]
		if n.rect_position.distance_to(old_pos) < 4.0:
			n.rect_position = new_pos
			n.rect_rotation = new_rot


# ---------- Angka fusion berwarna di atas kartu (duel) ----------

func _build_legend():
	_legend_panel = PanelContainer.new()
	_legend_panel.rect_position = Vector2(6, 96)
	_legend_panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_legend_panel.modulate = Color(1, 1, 1, 0.92)
	_legend_panel.visible = false
	_badge_layer.add_child(_legend_panel)
	_legend_box = VBoxContainer.new()
	_legend_box.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_legend_panel.add_child(_legend_box)


func _on_fn_toggle():
	_fn_on = not _fn_on
	_fn_btn.text = "Angka fusion duel: " + ("ON" if _fn_on else "OFF")
	if not _fn_on:
		_clear_badges()


func _on_lg_toggle():
	_lg_on = not _lg_on
	_lg_btn.text = "Legenda fusion: " + ("ON" if _lg_on else "OFF")
	_legend_panel.visible = _badges_shown and _lg_on and _legend_has


func _set_shown(flag):
	_badges_shown = flag
	_legend_panel.visible = flag and _lg_on and _legend_has


func _clear_badges():
	for b in _bg:
		if is_instance_valid(b["box"]):
			b["box"].queue_free()
	_bg = []
	_bsig = ""
	_legend_has = false
	_badges_shown = false
	_legend_panel.visible = false


func _is_prefix(a, b):
	if a.size() > b.size():
		return false
	for i in range(a.size()):
		if a[i] != b[i]:
			return false
	return true


# Ambil rangkaian terkuat; buang yang hanya potongan awal (atau perpanjangan) dari yang sudah dipilih.
func _select_chains(ranked, max_n):
	var selected = []
	for e in ranked:
		var skip = false
		for s in selected:
			if _is_prefix(e["srcs"], s["srcs"]) or _is_prefix(s["srcs"], e["srcs"]):
				skip = true
				break
		if skip:
			continue
		selected.append(e)
		if selected.size() >= max_n:
			break
	return selected


func _badge_tick():
	var scn = get_tree().current_scene
	if not _fn_on or scn == null or not scn.has_node("game_logic/player_logic"):
		if _bg.size() > 0 or _bsig != "":
			_clear_badges()
		return
	var gl = scn.get_node("game_logic")
	var phase = str(gl.GAME_PHASE)
	if not (phase in BADGE_PHASES):
		_set_shown(false)
		return
	_set_shown(true)

	var hand = []
	for id in gl.get_node("player_logic").player_hand:
		hand.append(str(id))
	var fields = []
	for i in range(5):
		var n = scn.get_node_or_null("duel_field/player_side_zones/monster_" + str(i))
		if n != null and n.is_visible():
			var fl = n.this_card_flags
			fields.append({"id": str(n.this_card_id), "ab": int(fl.get("atk_up", 0)), "db": int(fl.get("def_up", 0)), "slot": i})

	var sig = ""
	for h in hand:
		sig += h + ","
	sig += "|"
	for f in fields:
		sig += str(f["slot"]) + ":" + f["id"] + ":" + str(f["ab"]) + ":" + str(f["db"]) + ","
	if sig == _bsig:
		return
	_bsig = sig
	_rebuild_badges(scn, gl, hand, fields)


func _rebuild_badges(scn, gl, hand, fields):
	for b in _bg:
		if is_instance_valid(b["box"]):
			b["box"].queue_free()
	_bg = []

	var fi = _fusion_node(scn)
	if fi == null or hand.size() == 0:
		_update_legend([])
		return

	var old_phase = gl.GAME_PHASE
	gl.GAME_PHASE = "checking_for_fusions"
	var all = []
	for i in range(hand.size()):
		var out = []
		var cur = {"id": hand[i], "ab": 0, "db": 0}
		_dfs(fi, cur, hand, 1 << i, [], "", out, true, ["h" + str(i)])
		for e in out:
			all.append(e)
	for f in fields:
		var out2 = []
		var cur2 = {"id": f["id"], "ab": f["ab"], "db": f["db"]}
		_dfs(fi, cur2, hand, 0, [], "", out2, true, ["f" + str(f["slot"])])
		for e2 in out2:
			e2["key"] = "F" + str(f["slot"]) + "|" + e2["key"]
			all.append(e2)
	gl.GAME_PHASE = old_phase

	var selected = _select_chains(_rank(all), 4)

	# src -> daftar [indeks warna, nomor urut]
	var badges = {}
	for k in range(selected.size()):
		var srcs = selected[k]["srcs"]
		for p in range(srcs.size()):
			var sk = str(srcs[p])
			if not badges.has(sk):
				badges[sk] = []
			badges[sk].append([k, p + 1])

	for sk in badges.keys():
		var node = null
		if sk.begins_with("h"):
			node = scn.get_node_or_null("player_hand/card_" + sk.substr(1, sk.length() - 1))
		else:
			node = scn.get_node_or_null("duel_field/player_side_zones/monster_" + sk.substr(1, sk.length() - 1))
		if node == null:
			continue
		var box = Control.new()
		box.mouse_filter = Control.MOUSE_FILTER_IGNORE
		box.visible = false
		_badge_layer.add_child(box)
		var arr = badges[sk]
		var total = arr.size()
		for q in range(total):
			var c = _make_circle(FUSION_COLORS[arr[q][0]], str(arr[q][1]))
			c.rect_position = Vector2((total - 1 - q) * 37, 0)
			box.add_child(c)
		_bg.append({"node": node, "box": box, "w": total * 37 - 3})

	_update_legend(selected)


func _make_circle(color, text):
	var p = Panel.new()
	p.rect_min_size = Vector2(34, 34)
	p.rect_size = Vector2(34, 34)
	p.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var sb = StyleBoxFlat.new()
	sb.bg_color = color
	sb.set_corner_radius_all(17)
	sb.border_color = Color(1, 1, 1, 1)
	sb.set_border_width_all(3)
	p.add_stylebox_override("panel", sb)
	var l = Label.new()
	l.text = text
	l.align = Label.ALIGN_CENTER
	l.valign = Label.VALIGN_CENTER
	l.mouse_filter = Control.MOUSE_FILTER_IGNORE
	if _badge_font != null:
		l.add_font_override("font", _badge_font)
	l.add_color_override("font_color", Color(1, 1, 1, 1))
	p.add_child(l)
	l.rect_position = Vector2(0, 0)
	l.rect_size = Vector2(34, 34)
	return p


func _update_legend(selected):
	for c in _legend_box.get_children():
		_legend_box.remove_child(c)
		c.queue_free()
	_legend_has = selected.size() > 0
	if not _legend_has:
		_legend_panel.visible = false
		return
	var title = Label.new()
	title.text = "FUSION (angka = urutan kartu)"
	_legend_box.add_child(title)
	for k in range(selected.size()):
		var e = selected[k]
		var row = HBoxContainer.new()
		row.add_constant_override("separation", 6)
		var sw = ColorRect.new()
		sw.color = FUSION_COLORS[k]
		sw.rect_min_size = Vector2(16, 16)
		sw.size_flags_vertical = Control.SIZE_SHRINK_CENTER
		row.add_child(sw)
		var cnt = e["srcs"].size()
		var src_text = str(cnt) + " kartu"
		if str(e["srcs"][0]).begins_with("f"):
			src_text = "monster lapangan + " + str(cnt - 1) + " kartu tangan"
		var lb = Label.new()
		lb.text = FUSION_COLOR_NAMES[k] + ": " + _name(e["id"]) + " " + _stat_text(e["id"], e["ab"], e["db"]) + " - " + src_text
		row.add_child(lb)
		_legend_box.add_child(row)
	_legend_panel.visible = _badges_shown and _lg_on


# Lencana mengikuti kartu tiap frame (kartu bergerak, kamera lapangan bergeser)
func _process(_delta):
	if _list_btn != null and _list_btn.visible:
		var lp = _toggle.rect_position + Vector2(-60, 0)
		if lp.x < 0:
			lp.x = _toggle.rect_position.x + 60
		_list_btn.rect_position = lp
	_ef_update(_delta)
	if _fx_job != null and _ls_root.visible:
		_fx_step()
	if _bg.size() == 0:
		return
	for b in _bg:
		var n = b["node"]
		var box = b["box"]
		if not _badges_shown or not is_instance_valid(n) or not n.is_visible_in_tree():
			box.visible = false
			continue
		var xf = n.get_global_transform_with_canvas()
		box.rect_position = xf.xform(Vector2(n.rect_size.x, 0)) + Vector2(-b["w"] - 2, -20)
		box.visible = true


# ---------- Konfigurasi & tombol MOD yang bisa digeser ----------

func _cfg_load():
	var cf = ConfigFile.new()
	if cf.load("user://mod_menu.cfg") == OK:
		if cf.has_section_key("btn", "x"):
			_btn_pos = Vector2(float(cf.get_value("btn", "x", 0)), float(cf.get_value("btn", "y", 0)))
			_btn_saved = true
		_list_mode = bool(cf.get_value("view", "list_mode", true))
		_duel_all = bool(cf.get_value("view", "duel_all", false))
		_ef_on = bool(cf.get_value("view", "ef_on", true))


func _cfg_save():
	var cf = ConfigFile.new()
	cf.set_value("btn", "x", _toggle.rect_position.x)
	cf.set_value("btn", "y", _toggle.rect_position.y)
	cf.set_value("view", "list_mode", _list_mode)
	cf.set_value("view", "duel_all", _duel_all)
	cf.set_value("view", "ef_on", _ef_on)
	cf.save("user://mod_menu.cfg")


# Ketuk = buka menu. Tahan 0,35 detik lalu seret = pindahkan tombol.
func _on_toggle_input(event):
	if event is InputEventMouseButton and event.button_index == BUTTON_LEFT:
		if event.pressed:
			_tg_down = true
			_tg_dragging = false
			_tg_press_ms = OS.get_ticks_msec()
			_tg_grab = event.global_position - _toggle.rect_position
		else:
			if _tg_down:
				if _tg_dragging:
					_toggle.modulate = Color(1, 1, 1, 0.45)
					_cfg_save()
				elif OS.get_ticks_msec() - _tg_press_ms < 350:
					_on_toggle()
			_tg_down = false
			_tg_dragging = false
	elif event is InputEventMouseMotion and _tg_down:
		if not _tg_dragging and OS.get_ticks_msec() - _tg_press_ms >= 350:
			_tg_dragging = true
			_toggle.modulate = Color(1, 1, 1, 0.95)
		if _tg_dragging:
			var vp = get_viewport().get_visible_rect().size
			var p = event.global_position - _tg_grab
			p.x = clamp(p.x, 0, vp.x - _toggle.rect_size.x)
			p.y = clamp(p.y, 0, vp.y - _toggle.rect_size.y)
			_toggle.rect_position = p


# ---------- Tambah / keluarkan kartu dari deck (aturan sama seperti game) ----------

func _play(sound):
	var sc = get_node_or_null("/root/SoundControl")
	if sc != null:
		sc.play_sound(sound)


func _can_add(id):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null or not pd.player_trunk.has(id):
		return false
	var indeck = pd.player_deck.count(id)
	return int(pd.player_trunk[id]) - indeck > 0 and pd.player_deck.size() < DECK_LIMIT and indeck < 3


func _deck_add(id):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null or not _can_add(id):
		_play("poc_unable")
		return false
	pd.player_deck.append(id)
	_play("poc_move")
	_refresh_deck_ui()
	return true


func _deck_remove(id):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null or not pd.player_deck.has(id):
		return false
	pd.player_deck.erase(id)
	_play("poc_move")
	_refresh_deck_ui()
	return true


# Sama seperti yang dilakukan game setelah menambah/mengeluarkan kartu.
func _refresh_deck_ui():
	var scn = get_tree().current_scene
	var pd = get_node_or_null("/root/PlayerData")
	if scn != null and pd != null and str(scn.filename).ends_with("deck_building.tscn") and scn.has_node("panel_left/sortables"):
		var sorter = scn.get_node("panel_left/sortables")
		var sorted_trunk = sorter.sort_cards(pd.player_trunk.keys(), sorter.last_sorted_type)
		scn.current_highlighted_card = null
		scn.update_left_panel(sorted_trunk)
		if pd.player_deck.size() <= 40:
			scn.update_right_panel()
		else:
			# panel kanan asli hanya punya 40 slot, jadi urutkan deck sendiri
			_sort_deck(sorter, pd)
			scn.get_node("user_interface/back_button").modulate = Color(1, 1, 1, 1)
			scn.get_node("panel_right/deck_buttons/button_go_duel").modulate = Color(1, 1, 1, 1)
	_ls_refresh()


func _kind_of(id):
	var inf = _info(id)
	if inf == null:
		return "?"
	var eff = inf.get("effect", [])
	if typeof(eff) == TYPE_ARRAY and eff.size() == 0:
		return "Normal"
	var attr = str(inf.get("attribute", ""))
	if attr == "spell":
		if str(inf.get("type", "")) == "ritual":
			return "Ritual"
		return "Spell"
	if attr == "trap":
		return "Trap"
	return "Efek"


# ---------- Tampilan daftar deck (gaya Forbidden Memories) ----------

func _ls_button(text, x, y, w, h, method, binds):
	var b = Button.new()
	b.text = text
	b.rect_position = Vector2(x, y)
	b.rect_size = Vector2(w, h)
	b.focus_mode = Control.FOCUS_NONE
	if _list_font != null:
		b.add_font_override("font", _list_font)
	b.connect("pressed", self, method, binds)
	_ls_root.add_child(b)
	return b



func _build_list():
	var vp = get_viewport().get_visible_rect().size
	var fd = load("res://_resources/fonts/BankGothic Md BT.ttf")
	if fd != null:
		_list_font = DynamicFont.new()
		_list_font.font_data = fd
		_list_font.size = 22
		_new_font = DynamicFont.new()
		_new_font.font_data = fd
		_new_font.size = 18
		_new_font.outline_size = 2
		_new_font.outline_color = Color(0, 0, 0, 1)
		_stamp_font = DynamicFont.new()
		_stamp_font.font_data = fd
		_stamp_font.size = 56
		_stamp_font.outline_size = 6
		_stamp_font.outline_color = Color(0, 0, 0, 1)
		_small_font = DynamicFont.new()
		_small_font.font_data = fd
		_small_font.size = 18

	_ls_root = Control.new()
	_ls_root.rect_min_size = vp
	_ls_root.rect_size = vp
	_ls_root.mouse_filter = Control.MOUSE_FILTER_STOP
	_ls_root.visible = false
	add_child(_ls_root)

	var bg = ColorRect.new()
	bg.color = Color(0.05, 0.06, 0.1, 1)
	bg.rect_size = vp
	bg.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_ls_root.add_child(bg)

	_ls_count = Label.new()
	_ls_count.rect_position = Vector2(12, 8)
	_ls_count.rect_size = Vector2(880, 30)
	_ls_count.clip_text = true
	if _list_font != null:
		_ls_count.add_font_override("font", _list_font)
	_ls_root.add_child(_ls_count)

	_ls_button("Kembali", 900, 6, 90, 34, "_on_ls_back", [])
	_ls_duel_btn = _ls_button("Duel", 996, 6, 84, 34, "_on_ls_duel", [])
	_ls_duel_btn.visible = false

	# tab halaman: Koleksi dan Deck dipisah seperti di game aslinya
	_ls_tab_btns["trunk"] = _ls_button("KOLEKSI", 12, 46, 150, 34, "_ls_set_page", ["trunk"])
	_ls_tab_btns["deck"] = _ls_button("DECK", 168, 46, 110, 34, "_ls_set_page", ["deck"])
	_ls_tab_btns["fusion"] = _ls_button("FUSION", 284, 46, 130, 34, "_ls_set_page", ["fusion"])

	# tombol sortir (hanya di halaman Koleksi)
	var kinds = ["name", "atk", "def", "type", "attr", "no", "new"]
	var labels = ["Nama", "ATK", "DEF", "Tipe", "Atribut", "No.", "New 0"]
	for i in range(kinds.size()):
		_ls_sort_btns[kinds[i]] = _ls_button(labels[i], 330 + i * 88, 46, 84, 34, "_ls_sort", [kinds[i]])

	# kontrol tab FUSION: ATK/DEF, batas nilai, panjang rantai, tampilkan kartu tanpa fusion, buang semua
	_fx_stat_btn = _fx_btn("ATK", 430, 64, "_fx_toggle_stat", [])
	var c1 = _fx_btn("-500", 498, 72, "_fx_adj", [-500])
	var c2 = _fx_btn("-100", 574, 72, "_fx_adj", [-100])
	_fx_val_lbl = Label.new()
	_fx_val_lbl.text = str(_fx_val)
	_fx_val_lbl.rect_position = Vector2(650, 46)
	_fx_val_lbl.rect_size = Vector2(84, 34)
	_fx_val_lbl.align = Label.ALIGN_CENTER
	_fx_val_lbl.valign = Label.VALIGN_CENTER
	_fx_val_lbl.mouse_filter = Control.MOUSE_FILTER_IGNORE
	if _list_font != null:
		_fx_val_lbl.add_font_override("font", _list_font)
	_ls_root.add_child(_fx_val_lbl)
	var c3 = _fx_btn("+100", 738, 72, "_fx_adj", [100])
	var c4 = _fx_btn("+500", 814, 72, "_fx_adj", [500])
	_fx_len_btn = _fx_btn("Rantai: 2", 892, 104, "_fx_toggle_len", [])
	_fx_nf_btn = _fx_btn("Tanpa fusion", 1002, 136, "_fx_toggle_nf", [])
	_fx_bulk_btn = _fx_btn("Buang semua", 1142, 120, "_fx_bulk", [])
	_fx_ctrl = [_fx_stat_btn, c1, c2, _fx_val_lbl, c3, c4, _fx_len_btn, _fx_nf_btn, _fx_bulk_btn]

	_ls_header()
	_fx_header()
	_fx_info = _fx_heads[0]
	_fx_head_best = _fx_heads[2]
	_ls_make_list("trunk")
	_ls_make_list("deck")
	_ls_make_list("fusion")
	_ls_set_page("trunk")



func _ls_header():
	var idx = [0, 2, 3, 4, 5, 6, 7]
	var ts = ["No.", "Nama", "ATK/DEF", "Lv", "Tipe", "Jenis", "Jml"]
	for i in range(idx.size()):
		var l = Label.new()
		l.text = ts[i]
		l.rect_position = Vector2(12 + COL_X[idx[i]], 86)
		l.rect_size = Vector2(COL_W[idx[i]], 24)
		l.modulate = Color(1, 0.9, 0.5, 1)
		l.mouse_filter = Control.MOUSE_FILTER_IGNORE
		if _list_font != null:
			l.add_font_override("font", _list_font)
		_ls_root.add_child(l)
		_ls_heads.append(l)



func _ls_make_list(key):
	var sc = ScrollContainer.new()
	sc.rect_position = Vector2(12, 112)
	sc.rect_min_size = Vector2(LIST_W, 600)
	sc.rect_size = Vector2(LIST_W, 600)
	sc.scroll_horizontal_enabled = false
	_ls_root.add_child(sc)
	var spacer = Control.new()
	spacer.rect_min_size = Vector2(LIST_W - 14, ROW_H)
	spacer.mouse_filter = Control.MOUSE_FILTER_IGNORE
	sc.add_child(spacer)
	var L = {"sc": sc, "spacer": spacer, "rows": [], "data": [], "down_scroll": 0}
	for i in range(LIST_POOL):
		if key == "fusion":
			L["rows"].append(_ls_make_frow(key, spacer))
		else:
			L["rows"].append(_ls_make_row(key, spacer))
	_ls_lists[key] = L
	sc.get_v_scrollbar().connect("value_changed", self, "_ls_on_scroll", [key])


func _ls_lbl(parent, x, w):
	var l = Label.new()
	l.rect_position = Vector2(x, 0)
	l.rect_size = Vector2(w, ROW_H - 4)
	l.valign = Label.VALIGN_CENTER
	l.clip_text = true
	l.mouse_filter = Control.MOUSE_FILTER_IGNORE
	if _list_font != null:
		l.add_font_override("font", _list_font)
	parent.add_child(l)
	return l



func _ls_make_row(key, parent):
	var b = Button.new()
	b.rect_size = Vector2(LIST_W - 18, ROW_H - 4)
	b.focus_mode = Control.FOCUS_NONE
	b.visible = false
	parent.add_child(b)
	var row = {"root": b, "idx": -1, "id": ""}
	row["no"] = _ls_lbl(b, COL_X[0], COL_W[0])
	var ic = TextureRect.new()
	ic.expand = true
	ic.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	ic.rect_position = Vector2(COL_X[1], 8)
	ic.rect_size = Vector2(COL_W[1], 32)
	ic.mouse_filter = Control.MOUSE_FILTER_IGNORE
	b.add_child(ic)
	row["icon"] = ic
	row["name"] = _ls_lbl(b, COL_X[2], COL_W[2])
	row["stat"] = _ls_lbl(b, COL_X[3], COL_W[3])
	row["level"] = _ls_lbl(b, COL_X[4], COL_W[4])
	row["type"] = _ls_lbl(b, COL_X[5], COL_W[5])
	row["kind"] = _ls_lbl(b, COL_X[6], COL_W[6])
	row["cnt"] = _ls_lbl(b, COL_X[7], COL_W[7])
	var nl = _ls_lbl(b, COL_X[2] + 200, 60)
	nl.text = "NEW"
	nl.visible = false
	nl.add_color_override("font_color", Color(1, 0.8, 0.1, 1))
	if _new_font != null:
		nl.add_font_override("font", _new_font)
	row["new"] = nl
	var ab = Button.new()
	ab.text = "+" if key == "trunk" else "-"
	ab.rect_position = Vector2(COL_X[8], 4)
	ab.rect_size = Vector2(COL_W[8], ROW_H - 12)
	ab.focus_mode = Control.FOCUS_NONE
	if _list_font != null:
		ab.add_font_override("font", _list_font)
	b.add_child(ab)
	row["act"] = ab
	b.connect("button_down", self, "_ls_row_down", [key])
	b.connect("pressed", self, "_ls_row_pressed", [key, row])
	ab.connect("pressed", self, "_ls_action", [key, row])
	return row


func _ls_on_scroll(_value, key):
	_ls_bind(key)



func _ls_bind(key):
	var L = _ls_lists[key]
	var sv = int(L["sc"].scroll_vertical)
	var first = int(sv / ROW_H)
	var data = L["data"]
	var off = int(L.get("off", 0))
	var count = data.size() - off
	var rows = L["rows"]
	for r in range(rows.size()):
		var idx = first + r
		var row = rows[r]
		if idx >= count:
			row["root"].visible = false
			row["idx"] = -1
			continue
		row["idx"] = idx
		row["root"].rect_position = Vector2(0, idx * ROW_H)
		row["root"].visible = true
		if key == "fusion":
			_ls_fill_frow(row, data[idx + off])
		else:
			_ls_fill_row(key, row, str(data[idx]))


func _ls_fill_row(key, row, id):
	row["id"] = id
	var pd = get_node_or_null("/root/PlayerData")
	var inf = _info(id)
	if id.is_valid_integer():
		row["no"].text = str(int(id))
	else:
		row["no"].text = id
	row["name"].text = _name(id)
	var attr = ""
	if inf != null:
		attr = str(inf.get("attribute", ""))
	row["icon"].texture = load("res://_resources/_attributes/" + attr + ".png") if attr != "" else null
	if _is_spelltrap(id):
		row["stat"].text = "-"
		row["level"].text = "-"
	else:
		row["stat"].text = str(_atk(id)) + " / " + str(_def(id))
		row["level"].text = str(inf.get("level", 0)) if inf != null else "-"
	if inf != null:
		row["type"].text = str(inf.get("type", "")).capitalize()
	else:
		row["type"].text = ""
	row["kind"].text = _kind_of(id)
	var owned = 0
	var indeck = 0
	if pd != null:
		indeck = pd.player_deck.count(id)
		if pd.player_trunk.has(id):
			owned = int(pd.player_trunk[id])
	if key == "trunk":
		var avail = owned - indeck
		row["cnt"].text = "x" + str(avail)
		row["root"].modulate = Color(1, 1, 1, 1) if avail > 0 else Color(1, 1, 1, 0.45)
		row["act"].disabled = not _can_add(id)
		var isn = _is_new(id)
		row["new"].visible = isn
		if isn:
			var tw = 0.0
			if _list_font != null:
				tw = _list_font.get_string_size(str(row["name"].text)).x
			row["new"].rect_position = Vector2(COL_X[2] + min(tw, 320.0) + 8, 0)
	else:
		row["new"].visible = false
		row["cnt"].text = "x" + str(indeck)
		row["root"].modulate = Color(1, 1, 1, 1)
		row["act"].disabled = false


func _cmp_id(a, b):
	return int(a) < int(b)


func _ls_refresh():
	var scn = get_tree().current_scene
	var pd = get_node_or_null("/root/PlayerData")
	if scn == null or pd == null or not scn.has_node("panel_left/sortables"):
		return
	var sorter = scn.get_node("panel_left/sortables")
	var ids = []
	var news = _new_ids()
	if _ls_new_sort and news.size() == 0:
		_ls_new_sort = false
	if _ls_new_sort:
		var base = sorter.sort_cards(pd.player_trunk.keys(), "name")
		for nid in news:
			if pd.player_trunk.has(nid):
				ids.append(nid)
		for bid in base:
			if not news.has(str(bid)):
				ids.append(bid)
	elif _ls_no_sort:
		for k in pd.player_trunk.keys():
			ids.append(str(k))
		ids.sort_custom(self, "_cmp_id")
	else:
		ids = sorter.sort_cards(pd.player_trunk.keys(), sorter.last_sorted_type)
		if typeof(ids) != TYPE_ARRAY or ids.size() == 0:
			ids = pd.player_trunk.keys()
	_ls_lists["trunk"]["data"] = ids
	_ls_lists["deck"]["data"] = pd.player_deck.duplicate()
	for key in ["trunk", "deck"]:
		var L = _ls_lists[key]
		L["spacer"].rect_min_size = Vector2(LIST_W - 14, max(ROW_H, L["data"].size() * ROW_H))
		_ls_bind(key)
	if _ls_page == "fusion":
		_fx_ensure(false)

	var counts = {"Normal": 0, "Efek": 0, "Spell": 0, "Trap": 0, "Ritual": 0}
	for d in pd.player_deck:
		var kd = _kind_of(str(d))
		if counts.has(kd):
			counts[kd] += 1
	_ls_count.text = "DECK " + str(pd.player_deck.size()) + "/" + str(DECK_LIMIT) + "   Normal " + str(counts["Normal"]) + "  Efek " + str(counts["Efek"]) + "  Spell " + str(counts["Spell"]) + "  Trap " + str(counts["Trap"]) + "  Ritual " + str(counts["Ritual"])

	var active = str(sorter.last_sorted_type)
	if _ls_no_sort:
		active = "no"
	if _ls_new_sort:
		active = "new"
	for kk in _ls_sort_btns.keys():
		if kk == "new":
			_ls_sort_btns[kk].text = "New " + str(news.size())
			if kk == active:
				_ls_sort_btns[kk].modulate = Color(1, 1, 1, 1)
			elif news.size() > 0:
				_ls_sort_btns[kk].modulate = Color(1, 0.85, 0.3, 1)
			else:
				_ls_sort_btns[kk].modulate = Color(1, 1, 1, 0.4)
		else:
			_ls_sort_btns[kk].modulate = Color(1, 1, 1, 1) if kk == active else Color(1, 1, 1, 0.55)


func _ls_sort(kind):
	var scn = get_tree().current_scene
	if scn == null or not scn.has_node("panel_left/sortables"):
		return
	var sorter = scn.get_node("panel_left/sortables")
	if kind == "new" and _new_ids().size() == 0:
		_play("poc_unable")
		return
	_ls_no_sort = false
	_ls_new_sort = false
	if kind == "name":
		sorter._on_byNAME_button_up()
	elif kind == "atk":
		sorter._on_byATK_button_up()
	elif kind == "def":
		sorter._on_byDEF_button_up()
	elif kind == "type":
		sorter._on_byTYPE_button_up()
	elif kind == "attr":
		sorter._on_byATTR_button_up()
	elif kind == "new":
		_ls_new_sort = true
	else:
		_ls_no_sort = true
	_ls_refresh()


func _ls_row_down(key):
	_ls_lists[key]["down_scroll"] = _ls_lists[key]["sc"].scroll_vertical


func _ls_row_pressed(key, row):
	var L = _ls_lists[key]
	if abs(L["sc"].scroll_vertical - L["down_scroll"]) > 8:
		return
	if row["idx"] < 0:
		return
	if key == "trunk":
		_mark_seen(str(row["id"]))
		_ls_bind("trunk")
	_show_preview(str(row["id"]))


func _ls_action(key, row):
	if row["idx"] < 0:
		return
	if key == "fusion":
		_fx_remove_card(row["entry"][0])
		return
	var id = str(row["id"])
	if key == "trunk":
		_mark_seen(id)
		_deck_add(id)
	else:
		_deck_remove(id)


func _on_ls_back():
	var scn = get_tree().current_scene
	if scn != null and scn.has_method("_on_back_button_button_up"):
		scn._on_back_button_button_up()


func _on_ls_duel():
	var scn = get_tree().current_scene
	var pd = get_node_or_null("/root/PlayerData")
	if scn == null or pd == null:
		return
	if pd.player_deck.size() == 40:
		if scn.has_method("_on_go_duel_button_up"):
			scn._on_go_duel_button_up()
	elif pd.player_deck.size() > 40:
		# game aslinya hanya menerima tepat 40, jadi lakukan transisi yang sama secara langsung
		scn.button_click_animation("panel_right/deck_buttons/button_go_duel")
		scn.get_node("scene_transitioner").scene_transition("duel_scene")
	else:
		_play("poc_unable")


func _on_ls_mode():
	_list_mode = not _list_mode
	_ls_mode_btn.text = "Tampilan deck: " + ("DAFTAR" if _list_mode else "ASLI")
	_cfg_save()
	_ls_tick()


func _ls_tick():
	var pdx = get_node_or_null("/root/PlayerData")
	if pdx != null and str(pdx.going_to_duel) != "" and str(pdx.scene_to_return_after_duel) == "game_dialog":
		_last_story_opp = str(pdx.going_to_duel)
	_reward_tick()
	_menu_tick()
	if _bulk_armed_ms != -100000 and OS.get_ticks_msec() - _bulk_armed_ms > 5000:
		_bulk_armed_ms = -100000
		_fx_bulk_btn.text = "Buang semua"
	var scn = get_tree().current_scene
	if scn != null and scn.has_node("game_logic/player_logic"):
		_trim_duel_deck(scn.get_node("game_logic/player_logic"))
	var in_deck = false
	if scn != null and str(scn.filename).ends_with("deck_building.tscn") and scn.has_node("panel_left/sortables"):
		var pd = get_node_or_null("/root/PlayerData")
		in_deck = pd != null and pd.game_loaded == true
	_list_btn.visible = in_deck
	if in_deck and _list_mode:
		if not _ls_root.visible:
			_ls_root.visible = true
			_ls_refresh()
		var gb = scn.get_node_or_null("panel_right/deck_buttons/button_go_duel")
		_ls_duel_btn.visible = gb != null and gb.is_visible_in_tree()
	elif _ls_root.visible:
		_ls_root.visible = false
	_list_btn.text = "ASLI" if (in_deck and _list_mode) else "LIST"


# ---------- Popup: deskripsi, aksi, dan 4 fusion terkuat ----------

func _card_desc(id):
	if _ct == null:
		var s = load("res://_scripts/card_text.gd")
		if s != null:
			_ct = s.new()
	if _ct != null:
		var t = _ct.get_card_text(id)
		if t != null and str(t) != "":
			return str(t)
	var ib = _info_box()
	if ib != null:
		var dl = ib.get_node_or_null("card_text/Container/description_line1")
		if dl != null and str(dl.text) != "":
			return str(dl.text)
	return "-"


func _pv_update_actions(id):
	var scn = get_tree().current_scene
	var ok = scn != null and str(scn.filename).ends_with("deck_building.tscn")
	_pv_act_row.visible = ok
	if not ok:
		return
	var pd = get_node_or_null("/root/PlayerData")
	_pv_add_btn.disabled = not _can_add(id)
	_pv_rem_btn.disabled = pd == null or not pd.player_deck.has(id)


func _on_pv_add():
	if _pv_id == "":
		return
	_deck_add(_pv_id)
	_show_preview(_pv_id)


func _on_pv_rem():
	if _pv_id == "":
		return
	_deck_remove(_pv_id)
	_show_preview(_pv_id)


func _on_pv_add_partner(pid):
	_deck_add(pid)
	_show_preview(_pv_id)


# Semua hasil fusion monster dari kartu 'id' dengan kartu milikmu, diurutkan dari ATK terbesar.
func _top_fusions(id):
	var pd = get_node_or_null("/root/PlayerData")
	var out = {"list": [], "equips": 0}
	if pd == null:
		return out
	var sig = str(pd.player_trunk.size())
	if sig != _fx_sig:
		_fx_cache = {}
		_fx_sig = sig
	if _fx_cache.has(id):
		return _fx_cache[id]
	var fi = _fusion_node(get_tree().current_scene)
	if fi == null:
		return out
	var cand = {}
	var equips = 0
	for k in pd.player_trunk.keys():
		var oid = str(k)
		var owned = int(pd.player_trunk[k])
		if owned <= 0:
			continue
		if oid == id and owned < 2:
			continue
		for order in range(2):
			if order == 1 and oid == id:
				continue
			var a = id
			var b = oid
			if order == 1:
				a = oid
				b = id
			var r = _fuse(fi, a, b)
			if r == null:
				continue
			if r[1] != null:
				equips += 1
				continue
			var nid = r[0]
			if not cand.has(nid):
				cand[nid] = {"res": nid, "score": _atk(nid), "partners": []}
			if not cand[nid]["partners"].has(oid):
				cand[nid]["partners"].append(oid)
	var arr = []
	for rk in cand.keys():
		arr.append(cand[rk])
	arr.sort_custom(self, "_cmp_score")
	out["list"] = arr
	out["equips"] = equips
	_fx_cache[id] = out
	return out


# Pilih partner terbaik: yang sudah di deck dulu, lalu yang masih tersedia.
func _pick_partner(id, partners, pd):
	var best = ""
	var best_score = -1
	for p in partners:
		var need = 1
		if p == id:
			need = 2
		var indeck = pd.player_deck.count(p)
		var owned = 0
		if pd.player_trunk.has(p):
			owned = int(pd.player_trunk[p])
		var sc = 0
		if indeck >= need:
			sc = 2
		elif owned - indeck > 0:
			sc = 1
		if sc > best_score:
			best_score = sc
			best = p
	return best


func _fill_fusion_rows(id):
	for c in _pv_fusion_box.get_children():
		_pv_fusion_box.remove_child(c)
		c.queue_free()
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null:
		return
	var scn = get_tree().current_scene
	var in_deck_scene = scn != null and str(scn.filename).ends_with("deck_building.tscn")
	var res = _top_fusions(id)
	var list = res["list"]
	if list.size() == 0:
		var none = Label.new()
		none.text = "Tidak ada fusion dengan kartu yang kamu miliki."
		none.autowrap = true
		none.rect_min_size = Vector2(600, 0)
		_pv_fusion_box.add_child(none)
		return
	var shown = min(list.size(), 4)
	for i in range(shown):
		var f = list[i]
		var pid = _pick_partner(id, f["partners"], pd)
		var row = HBoxContainer.new()
		row.add_constant_override("separation", 8)
		_pv_fusion_box.add_child(row)
		var lb = Label.new()
		lb.autowrap = true
		lb.rect_min_size = Vector2(430, 0)
		lb.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		lb.text = _step_label(id, pid, f["res"], null) + " " + _stat_text(f["res"], 0, 0)
		row.add_child(lb)
		var need = 1
		if pid == id:
			need = 2
		var indeck = pd.player_deck.count(pid)
		var owned = 0
		if pd.player_trunk.has(pid):
			owned = int(pd.player_trunk[pid])
		if indeck >= need:
			var st = Label.new()
			st.text = "Sudah di deck"
			st.rect_min_size = Vector2(150, 0)
			row.add_child(st)
		elif owned - indeck > 0 and in_deck_scene:
			var ab = Button.new()
			ab.text = "+ Tambah"
			ab.rect_min_size = Vector2(150, 34)
			ab.disabled = not _can_add(pid)
			ab.connect("pressed", self, "_on_pv_add_partner", [pid])
			row.add_child(ab)
		else:
			var st2 = Label.new()
			st2.text = "Ada di koleksi"
			st2.rect_min_size = Vector2(150, 0)
			row.add_child(st2)
	if list.size() > shown:
		var more = Label.new()
		more.text = "(+" + str(list.size() - shown) + " fusion lain yang lebih lemah)"
		more.rect_min_size = Vector2(600, 0)
		_pv_fusion_box.add_child(more)
	if res["equips"] > 0:
		var eq = Label.new()
		eq.text = "(+" + str(res["equips"]) + " kombinasi equip/boost)"
		eq.rect_min_size = Vector2(600, 0)
		_pv_fusion_box.add_child(eq)


# ---------- Kartu baru (NEW) ----------

# Kartu hadiah/toko tercatat di PlayerData.last_reward_cards (ikut tersimpan di save game).
# NEW = kartu yang baru pertama kali dimiliki: jumlah miliknya tidak lebih dari jumlah catatannya di daftar itu.
func _is_new(id):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null:
		return false
	var cnt = pd.last_reward_cards.count(id)
	if cnt <= 0:
		return false
	var owned = 0
	if pd.player_trunk.has(id):
		owned = int(pd.player_trunk[id])
	return owned <= cnt


# Kartu NEW, yang terbaru dulu.
func _new_ids():
	var pd = get_node_or_null("/root/PlayerData")
	var out = []
	if pd == null:
		return out
	var seen = {}
	var lst = pd.last_reward_cards
	for i in range(lst.size() - 1, -1, -1):
		var id = str(lst[i])
		if seen.has(id):
			continue
		seen[id] = true
		if _is_new(id):
			out.append(id)
	return out


func _mark_seen(id):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null:
		return
	var changed = false
	while pd.last_reward_cards.has(id):
		pd.last_reward_cards.erase(id)
		changed = true
	if changed:
		_save()


# Urutan deck seperti yang dilakukan game (ATK, lalu Normal, Efek, Spell, Trap, Ritual).
func _sort_deck(sorter, pd):
	var sorted_deck = sorter.sort_with_duplicates(pd.player_deck, "atk")
	var buckets = {"Normal": [], "Efek": [], "Spell": [], "Trap": [], "Ritual": []}
	for d in sorted_deck:
		var kd = _kind_of(str(d))
		if buckets.has(kd):
			buckets[kd].append(d)
	pd.player_deck = buckets["Normal"] + buckets["Efek"] + buckets["Spell"] + buckets["Trap"] + buckets["Ritual"]


# Layar hadiah duel: setelah kartu berbalik, beri tulisan NEW! bila kartu itu baru pertama kali didapat.
func _reward_tick():
	var scn = get_tree().current_scene
	if scn == null or not scn.has_node("cards_reward/HBoxContainer/reward_1"):
		return
	var ph = scn.get("phase_of_reveal")
	if typeof(ph) != TYPE_INT:
		return
	for k in range(1, 4):
		if ph < 3 + k:
			continue
		var n = scn.get_node_or_null("cards_reward/HBoxContainer/reward_" + str(k))
		if n == null or n.has_node("ModNewTag"):
			continue
		var cb = n.get_node_or_null("card_design/card_back")
		if cb == null or cb.visible:
			continue
		var id = str(n.this_card_id)
		if id == "" or not _is_new(id):
			continue
		var l = Label.new()
		l.name = "ModNewTag"
		l.text = "NEW!"
		l.align = Label.ALIGN_CENTER
		l.valign = Label.VALIGN_CENTER
		l.rect_position = Vector2(0, 80)
		l.rect_size = Vector2(330, 64)
		l.mouse_filter = Control.MOUSE_FILTER_IGNORE
		if _stamp_font != null:
			l.add_font_override("font", _stamp_font)
		l.add_color_override("font_color", Color(1, 0.82, 0.1, 1))
		n.add_child(l)


# ---------- Halaman Koleksi / Deck ----------


func _ls_set_page(page):
	_ls_page = page
	for k in _ls_lists.keys():
		_ls_lists[k]["sc"].visible = (k == page)
	for kk in _ls_sort_btns.keys():
		_ls_sort_btns[kk].visible = (page == "trunk")
	for c in _fx_ctrl:
		c.visible = (page == "fusion")
	for h in _ls_heads:
		h.visible = (page != "fusion")
	for h2 in _fx_heads:
		h2.visible = (page == "fusion")
	_fx_info.visible = (page == "fusion")
	for tk in _ls_tab_btns.keys():
		_ls_tab_btns[tk].modulate = Color(1, 1, 1, 1) if tk == page else Color(1, 1, 1, 0.5)
	if page == "fusion":
		_fx_ensure(true)
	else:
		_ls_bind(page)


func _on_duel_mode():
	_duel_all = not _duel_all
	_duel_btn.text = "Deck saat duel: " + ("SEMUA" if _duel_all else "40 acak")
	_cfg_save()
	_say("Berlaku di duel berikutnya.")


# Deck yang tersimpan tidak diubah; hanya salinan milik duel yang dipotong jadi 40 kartu acak.
func _trim_duel_deck(pl):
	if _duel_all or not is_instance_valid(pl):
		return
	if pl.has_meta("mod_trimmed"):
		return
	pl.set_meta("mod_trimmed", true)
	if pl.player_hand.size() > 0:
		return
	if pl.player_deck.size() > 40:
		pl.player_deck.resize(40)


# ---------- Tab FUSION: semua fusion monster, disaring dengan batas ATK/DEF ----------



func _fx_btn(text, x, w, method, binds):
	var b = _ls_button(text, x, 46, w, 34, method, binds)
	if _small_font != null:
		b.add_font_override("font", _small_font)
	return b


func _fx_header():
	var xs = [8, 316, 524, 862, 1080]
	var ws = [330, 200, 330, 200, 120]
	var ts = ["Kartu di deck", "ATK / DEF kartu", "Fusion terkuat (2 kartu)", "ATK / DEF hasil", "Aksi"]
	for i in range(xs.size()):
		var l = Label.new()
		l.text = ts[i]
		l.rect_position = Vector2(12 + xs[i], 86)
		l.rect_size = Vector2(ws[i], 24)
		l.clip_text = true
		l.modulate = Color(1, 0.9, 0.5, 1)
		l.mouse_filter = Control.MOUSE_FILTER_IGNORE
		if _small_font != null:
			l.add_font_override("font", _small_font)
		_ls_root.add_child(l)
		_fx_heads.append(l)



func _ls_make_frow(key, parent):
	var b = Button.new()
	b.rect_size = Vector2(LIST_W - 18, ROW_H - 4)
	b.focus_mode = Control.FOCUS_NONE
	b.visible = false
	parent.add_child(b)
	var row = {"root": b, "idx": -1, "id": "", "entry": null}
	row["name"] = _ls_lbl(b, 8, 300)
	row["own"] = _ls_lbl(b, 316, 200)
	row["bres"] = _ls_lbl(b, 524, 330)
	row["bstat"] = _ls_lbl(b, 862, 200)
	for fk in ["name", "own", "bres", "bstat"]:
		if _small_font != null:
			row[fk].add_font_override("font", _small_font)
	var ab = Button.new()
	ab.text = "Buang"
	ab.rect_position = Vector2(1080, 4)
	ab.rect_size = Vector2(128, ROW_H - 12)
	ab.focus_mode = Control.FOCUS_NONE
	if _list_font != null:
		ab.add_font_override("font", _list_font)
	b.add_child(ab)
	row["act"] = ab
	b.connect("button_down", self, "_ls_row_down", [key])
	b.connect("pressed", self, "_ls_row_pressed", [key, row])
	ab.connect("pressed", self, "_ls_action", [key, row])
	return row


func _ls_fill_frow(row, e):
	row["entry"] = e
	row["id"] = e[0]
	var nm = _name(e[0])
	if int(e[7]) > 1:
		nm += " x" + str(e[7])
	row["name"].text = nm
	row["own"].text = "ATK " + str(e[5]) + " / DEF " + str(e[6])
	var bi = 1
	if _fx_stat == "def":
		bi = 3
	if e[bi] < 0:
		row["bres"].text = "tanpa fusion"
		row["bstat"].text = ""
		row["root"].modulate = Color(1, 1, 1, 0.6)
	else:
		var rid = e[bi + 1]
		row["bres"].text = "-> " + _name(rid)
		row["bstat"].text = "ATK " + str(_atk(rid)) + " / DEF " + str(_def(rid))
		row["root"].modulate = Color(1, 1, 1, 1)


func _fx_toggle_stat():
	_fx_stat = "def" if _fx_stat == "atk" else "atk"
	_fx_stat_btn.text = _fx_stat.to_upper()
	_fx_render(true)


func _fx_adj(delta):
	_fx_val = int(clamp(_fx_val + delta, 0, 9999))
	_fx_val_lbl.text = str(_fx_val)
	_fx_render(true)




func _fx_sig_for(pd):
	var d = pd.player_deck.duplicate()
	d.sort()
	var t = str(_fx_len) + "|"
	for x in d:
		t += str(x) + ","
	return t


func _fx_ensure(reset_scroll):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null or pd.game_loaded != true:
		_fx_info.text = "Muat game dulu."
		return
	var sig = _fx_sig_for(pd)
	if _fx_store["done"] and _fx_store["sig"] == sig:
		_fx_render(reset_scroll)
		return
	if _fx_job != null and _fx_job["sig"] == sig:
		return
	_fx_start(sig, pd)



func _fx_start(sig, pd):
	var fi = _fusion_node(get_tree().current_scene)
	if fi == null:
		_fx_info.text = "Gagal memuat fusion."
		return
	var counts = {}
	var ids = []
	for d in pd.player_deck:
		var sid = str(d)
		if counts.has(sid):
			counts[sid] += 1
		else:
			counts[sid] = 1
			ids.append(sid)
	var mons = []
	for id in ids:
		if _info(id) != null and not _is_spelltrap(id):
			mons.append(id)
	var dist = {}
	for m in mons:
		dist[m] = 0
	_fx_job = {"sig": sig, "fi": fi, "M": mons, "cnt": counts, "queue": mons.duplicate(), "dist": dist, "trans": {}, "qi": 0, "pi": 0, "memo": {}, "last_ui": 0, "maxp": _fx_len - 1}
	_ls_lists["fusion"]["data"] = []
	_ls_lists["fusion"]["off"] = 0
	_ls_lists["fusion"]["spacer"].rect_min_size = Vector2(LIST_W - 14, ROW_H)
	_ls_bind("fusion")
	_fx_info.text = "Menghitung..."



# Hitung fusion monster dari tiap kartu deck: pasangan (2 kartu) atau rantai 3 kartu.
# Dipanggil tiap frame; kerja dibatasi sekitar 12 ms supaya layar tetap lancar.
func _fx_step():
	var job = _fx_job
	var M = job["M"]
	var t0 = OS.get_ticks_msec()
	while OS.get_ticks_msec() - t0 < 12:
		if job["qi"] >= job["queue"].size():
			_fx_finish()
			return
		var X = job["queue"][job["qi"]]
		var k = job["dist"][X]
		if k > job["maxp"] - 1 or job["pi"] >= M.size():
			job["qi"] += 1
			job["pi"] = 0
			continue
		var P = M[job["pi"]]
		job["pi"] += 1
		if X == P and k == 0 and int(job["cnt"][P]) < 2:
			continue
		var r = _fuse(job["fi"], X, P)
		if r == null or r[1] != null:
			continue
		var R = r[0]
		if not job["trans"].has(X):
			job["trans"][X] = []
		job["trans"][X].append([P, R])
		if not job["dist"].has(R):
			job["dist"][R] = k + 1
			job["queue"].append(R)
	if OS.get_ticks_msec() - job["last_ui"] > 250:
		job["last_ui"] = OS.get_ticks_msec()
		_fx_info.text = "Menghitung " + str(job["qi"]) + "/" + str(job["queue"].size())




func _fx_finish():
	var job = _fx_job
	var M = job["M"]
	var pot = {}
	for c in M:
		pot[c] = [-1, "", -1, ""]
	# tiap rangkaian memakai kartu yang berbeda (jumlah salinan di deck dihormati);
	# semua kartu dalam rangkaian mendapat nilai hasilnya
	for s in M:
		var used = {s: 1}
		_fx_dfs(job, s, [s], used, 0, job["maxp"], pot)
	var rows = []
	for c in M:
		var p = pot[c]
		rows.append([c, p[0], p[1], p[2], p[3], _atk(c), _def(c), int(job["cnt"][c])])
	_fx_store = {"sig": job["sig"], "res": rows, "done": true}
	_fx_job = null
	if _ls_page == "fusion":
		_fx_render(not _fx_keep_scroll)
	_fx_keep_scroll = false


func _fx_render(reset_scroll):
	if _ls_page != "fusion" or not _fx_store["done"]:
		return
	var L = _ls_lists["fusion"]
	var bi = 1
	if _fx_stat == "def":
		bi = 3
	var have = []
	var none = []
	for e in _fx_store["res"]:
		if e[bi] < 0:
			none.append(e)
		elif e[bi] <= _fx_val:
			have.append(e)
	if _fx_stat == "atk":
		have.sort_custom(self, "_fx_cmp_have_atk")
		none.sort_custom(self, "_fx_cmp_none_atk")
	else:
		have.sort_custom(self, "_fx_cmp_have_def")
		none.sort_custom(self, "_fx_cmp_none_def")
	var data = have
	if _fx_show_nf:
		data = have + none
	L["data"] = data
	L["off"] = 0
	L["spacer"].rect_min_size = Vector2(LIST_W - 14, max(ROW_H, data.size() * ROW_H))
	if reset_scroll:
		L["sc"].scroll_vertical = 0
	_ls_bind("fusion")
	var txt = str(have.size()) + " kartu"
	if _fx_show_nf and none.size() > 0:
		txt += " +" + str(none.size())
	_fx_info.text = "Kartu di deck: " + txt


func _read_save_text():
	var f = File.new()
	if not f.file_exists("user://savegame.save"):
		return ""
	if f.open_encrypted_with_pass("user://savegame.save", File.READ, OS.get_unique_id()) != OK:
		return ""
	var t = f.get_as_text()
	f.close()
	return t.strip_edges()


func _on_save_export():
	var pd = get_node_or_null("/root/PlayerData")
	if pd != null and pd.game_loaded == true:
		_save()
	var text = _read_save_text()
	if text == "":
		_say("Belum ada file save. Masuk ke game (New / Load Game) dulu.")
		return
	var raw = text.to_utf8()
	var comp = raw.compress(File.COMPRESSION_DEFLATE)
	var code = "PMSAVE1:" + str(raw.size()) + ":" + text.md5_text().substr(0, 8) + ":" + Marshalls.raw_to_base64(comp)
	OS.clipboard = code
	if OS.clipboard == code:
		_say("Kode save disalin (" + str(code.length()) + " karakter). Tempel di aplikasi catatan lalu simpan.")
	else:
		_say("Clipboard tidak bisa dipakai di perangkat ini.")


func _on_save_import():
	var now = OS.get_ticks_msec()
	if now - _imp_armed_ms > 6000:
		_imp_armed_ms = now
		_say("Impor akan MENIMPA save sekarang (cadangan dibuat otomatis). Tekan lagi dalam 6 detik untuk lanjut.")
		return
	_imp_armed_ms = -100000
	var scn = get_tree().current_scene
	if scn != null and scn.has_node("game_logic"):
		_say("Impor tidak bisa dilakukan saat duel.")
		return
	var code = OS.clipboard.strip_edges()
	if not code.begins_with("PMSAVE1:"):
		_say("Clipboard tidak berisi kode save. Salin dulu kodenya.")
		return
	var parts = code.split(":", true, 3)
	if parts.size() < 4:
		_say("Kode save tidak lengkap.")
		return
	var raw = Marshalls.base64_to_raw(parts[3])
	var dec = raw.decompress(int(parts[1]), File.COMPRESSION_DEFLATE)
	var text = dec.get_string_from_utf8()
	if text.md5_text().substr(0, 8) != parts[2]:
		_say("Kode save rusak atau terpotong. Salin ulang kodenya.")
		return
	var parsed = JSON.parse(text)
	if parsed.error != OK or typeof(parsed.result) != TYPE_DICTIONARY or not parsed.result.has("player_trunk") or not parsed.result.has("player_deck"):
		_say("Isi kode save tidak valid.")
		return
	var d = Directory.new()
	if d.file_exists("user://savegame.save"):
		d.copy("user://savegame.save", "user://savegame_backup.save")
	var f = File.new()
	if f.open_encrypted_with_pass("user://savegame.save", File.WRITE, OS.get_unique_id()) != OK:
		_say("Gagal menulis file save.")
		return
	f.store_line(text)
	f.close()
	# muat ke memori seperti tombol Load Game
	var script = load("res://_scripts/save_load_logic.gd")
	var pd = get_node_or_null("/root/PlayerData")
	if script != null and pd != null:
		var sl = script.new()
		sl.load_game()
		sl.free()
		pd.game_loaded = true
	_fx_store["done"] = false
	_fx_cache = {}
	_say("Save berhasil diimpor dan dimuat.")
	get_tree().reload_current_scene()


# ---------- Penyaring kartu deck berdasarkan fusion terkuat ----------

func _fx_cmp_have_atk(a, b):
	return a[1] > b[1] or (a[1] == b[1] and a[5] > b[5])


func _fx_cmp_have_def(a, b):
	return a[3] > b[3] or (a[3] == b[3] and a[6] > b[6])


func _fx_cmp_none_atk(a, b):
	return a[5] > b[5]


func _fx_cmp_none_def(a, b):
	return a[6] > b[6]


func _fx_merge(dst, src):
	if src[0] > dst[0]:
		dst[0] = src[0]
		dst[1] = src[1]
	if src[2] > dst[2]:
		dst[2] = src[2]
		dst[3] = src[3]


# Hasil fusion terkuat (ATK dan DEF) yang bisa dicapai dari kartu 'state' dengan paling banyak d bahan lagi.
# Hasil: [ATK terbesar, id kartunya, DEF terbesar, id kartunya]; -1 bila tidak ada.
func _fx_toggle_nf():
	_fx_show_nf = not _fx_show_nf
	_fx_nf_btn.modulate = Color(1, 1, 1, 1) if _fx_show_nf else Color(1, 1, 1, 0.5)
	_fx_render(true)


func _fx_remove_card(id):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null:
		return
	while pd.player_deck.has(id):
		pd.player_deck.erase(id)
	_play("poc_move")
	_fx_keep_scroll = true
	_refresh_deck_ui()


# Buang semua kartu yang sedang tampil (semua salinannya). Tekan dua kali untuk memastikan.
func _fx_bulk():
	var pd = get_node_or_null("/root/PlayerData")
	var L = _ls_lists["fusion"]
	var data = L["data"]
	if pd == null or data.size() == 0:
		_fx_info.text = "Kosong"
		return
	var copies = 0
	for e in data:
		copies += int(e[7])
	var now = OS.get_ticks_msec()
	if now - _bulk_armed_ms > 5000:
		_bulk_armed_ms = now
		_fx_bulk_btn.text = "Yakin? " + str(copies)
		return
	_bulk_armed_ms = -100000
	_fx_bulk_btn.text = "Buang semua"
	for e in data:
		while pd.player_deck.has(e[0]):
			pd.player_deck.erase(e[0])
	_play("poc_move")
	_fx_keep_scroll = false
	_refresh_deck_ui()


func _fx_dfs(job, cur, parts, used, depth, maxp, pot):
	if depth >= maxp or not job["trans"].has(cur):
		return
	for t in job["trans"][cur]:
		var p = t[0]
		var r = t[1]
		if int(used.get(p, 0)) >= int(job["cnt"][p]):
			continue
		var v = [_atk(r), r, _def(r), r]
		var np = parts + [p]
		for x in np:
			_fx_merge(pot[x], v)
		used[p] = int(used.get(p, 0)) + 1
		_fx_dfs(job, r, np, used, depth + 1, maxp, pot)
		used[p] = int(used[p]) - 1


func _fx_toggle_len():
	_fx_len = 3 if _fx_len == 2 else 2
	_fx_len_btn.text = "Rantai: " + str(_fx_len)
	_fx_head_best.text = "Fusion terkuat (" + str(_fx_len) + " kartu)"
	_fx_ensure(true)


# ---------- Tombol Campaign & kalah tanpa menghapus progres ----------

func _menu_tick():
	var scn = get_tree().current_scene
	if scn == null:
		return
	var cb = scn.get_node_or_null("CenterContainer/VBoxContainer/btn_campaign")
	if cb != null and not cb.visible:
		cb.visible = true


# Dipanggil sebelum _ready layar Game Over, jadi data masih utuh.
func _take_snapshot():
	_snap = {}
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null:
		return
	for k in SNAP_KEYS:
		var v = pd.get(k)
		if typeof(v) == TYPE_ARRAY or typeof(v) == TYPE_DICTIONARY:
			v = v.duplicate(true)
		_snap[k] = v
	var flags = {}
	var dl = get_node_or_null("/root/Dialogic")
	if dl != null:
		for f in DIALOGIC_FLAGS:
			flags[f] = dl.get_variable(f)
	_snap["_dialogic"] = flags


func _after_game_over(node):
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null or _snap.empty():
		return
	for k in SNAP_KEYS:
		pd.set(k, _snap[k])
	pd.game_loaded = true
	var dl = get_node_or_null("/root/Dialogic")
	if dl != null and _snap.has("_dialogic"):
		for f in _snap["_dialogic"].keys():
			dl.set_variable(f, _snap["_dialogic"][f])
	# hentikan hitung mundur layar Game Over supaya tidak pindah ke menu utama sendiri
	if is_instance_valid(node):
		var t = node.get_node_or_null("timer")
		if t != null:
			t.stop()
	_save()
	var sc = str(pd.scene_to_return_after_duel)
	var kind = "choice"
	if sc.begins_with("tournament"):
		kind = "tournament"
	elif sc == "game_dialog" and _last_story_opp != "":
		var flag = "campaign_defeat_" + _last_story_opp.to_upper()
		if not pd.recorded_campaign_defeats.has(flag):
			kind = "story"
	# penjaga: bab yang sama berakhir buruk berulang-ulang
	var last_dlg = ""
	if pd.recorded_dialogs.size() > 0:
		last_dlg = str(pd.recorded_dialogs[-1])
	if kind == "choice":
		if last_dlg == _go_last_dlg:
			_go_repeat += 1
		else:
			_go_repeat = 0
		_go_last_dlg = last_dlg
	else:
		_go_repeat = 0
		_go_last_dlg = ""
	_show_go(kind)


func _build_gameover():
	var vp = get_viewport().get_visible_rect().size
	_go_root = Control.new()
	_go_root.rect_min_size = vp
	_go_root.rect_size = vp
	_go_root.mouse_filter = Control.MOUSE_FILTER_STOP
	_go_root.visible = false
	add_child(_go_root)
	var dim = ColorRect.new()
	dim.color = Color(0, 0, 0, 0.75)
	dim.rect_size = vp
	dim.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_go_root.add_child(dim)
	_go_panel = PanelContainer.new()
	_go_panel.rect_scale = Vector2(1.3, 1.3)
	_go_panel.rect_position = Vector2(vp.x / 2 - 286, vp.y / 2 - 150)
	_go_root.add_child(_go_panel)
	var box = VBoxContainer.new()
	box.rect_min_size = Vector2(440, 0)
	box.add_constant_override("separation", 10)
	_go_panel.add_child(box)
	_go_title = _add_label(box, "")
	_go_title.rect_min_size = Vector2(440, 0)
	_go_text = _add_label(box, "")
	_go_text.rect_min_size = Vector2(440, 0)
	var row = HBoxContainer.new()
	row.add_constant_override("separation", 10)
	box.add_child(row)
	_go_btn1 = _add_btn(row, "", "_on_go1")
	_go_btn2 = _add_btn(row, "", "_on_go2")


func _show_go(kind):
	_go_kind = kind
	if kind == "story":
		_go_title.text = "KALAH!"
		_go_text.text = "Progres dan koleksimu aman. Mau mencoba lagi, atau lanjut ke bab berikutnya?"
		_go_btn1.text = "Ulangi duel"
		_go_btn2.text = "Lanjut cerita"
		_go_btn2.visible = true
	elif kind == "tournament":
		_go_title.text = "KALAH DI TURNAMEN"
		_go_text.text = "Progres turnamen direset, tapi deck dan koleksimu tetap aman."
		_go_btn1.text = "OK"
		_go_btn2.visible = false
	else:
		_go_title.text = "AKHIR YANG BURUK"
		_go_text.text = "Pilihanmu membawa akhir yang buruk. Kamu akan mengulang bab ini."
		_go_btn1.text = "Ulangi bab"
		_go_btn2.text = "Lewati bab ini"
		_go_btn2.visible = _go_repeat >= 1
		if _go_repeat >= 1:
			_go_text.text = "Bab ini berakhir buruk lagi. Ulangi bab, atau lewati bab ini supaya cerita tetap berjalan."
	_go_root.visible = true


func _go_goto(scene):
	var scn = get_tree().current_scene
	var st = null
	if scn != null:
		st = scn.get_node_or_null("scene_transitioner")
	if st != null and st.has_method("scene_transition"):
		st.scene_transition(scene)
	else:
		get_tree().change_scene("res://_scenes/" + scene + ".tscn")


func _on_go1():
	_go_root.visible = false
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null:
		return
	if _go_kind == "story":
		pd.last_duel_result = ""
		pd.going_to_duel = _last_story_opp
		pd.scene_to_return_after_duel = "game_dialog"
		_go_goto("deck_building")
	elif _go_kind == "tournament":
		pd.tournament_last_progression_saved = ""
		pd.tournament_competitors_saved = {"green": [], "blue": [], "purple": [], "red": [], "gold": []}
		pd.tournament_last_duelist_saved = ""
		pd.last_duel_result = ""
		pd.scene_to_return_after_duel = ""
		_save()
		_go_goto("main_menu")
	else:
		_go_goto("game_dialog")


# "Lanjut cerita": dianggap sudah mengalahkan lawan itu supaya cerita bisa berlanjut.
func _on_go2():
	_go_root.visible = false
	var pd = get_node_or_null("/root/PlayerData")
	if pd == null:
		return
	if _go_kind == "choice":
		var num = 0
		if pd.recorded_dialogs.size() > 0:
			num = int(str(pd.recorded_dialogs[-1]).split("_")[1])
		pd.recorded_dialogs.append("dlg_" + str(num + 1).pad_zeros(3))
		_go_repeat = 0
		_go_last_dlg = ""
		_save()
		_go_goto("game_dialog")
		return
	var flag = "campaign_defeat_" + _last_story_opp.to_upper()
	if not pd.recorded_campaign_defeats.has(flag):
		pd.recorded_campaign_defeats.append(flag)
	var dl = get_node_or_null("/root/Dialogic")
	if dl != null:
		dl.set_variable(flag, "true")
	pd.last_duel_result = ""
	_save()
	_go_goto("game_dialog")


# ---------- Efek kilatan dan api (tanpa getar layar) ----------

func _build_effects():
	var vp = get_viewport().get_visible_rect().size
	_ef_flash = ColorRect.new()
	_ef_flash.color = Color(1, 1, 1, 0)
	_ef_flash.rect_size = vp
	_ef_flash.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_ef_flash.visible = false
	_badge_layer.add_child(_ef_flash)


func _on_ef_toggle():
	_ef_on = not _ef_on
	_ef_btn.text = "Efek kilat/api: " + ("ON" if _ef_on else "OFF")
	_cfg_save()


func _ef_flash_now(color, alpha):
	_ef_flash_col = color
	if alpha > _ef_flash_a:
		_ef_flash_a = alpha


func _ef_update(delta):
	if _ef_flash_a > 0.0:
		_ef_flash_a = max(0.0, _ef_flash_a - delta * 2.2)
		_ef_flash.color = Color(_ef_flash_col.r, _ef_flash_col.g, _ef_flash_col.b, _ef_flash_a)
		_ef_flash.visible = _ef_flash_a > 0.01
	var keep = []
	for it in _ef_items:
		it["t"] += delta
		if it["t"] >= it["life"] or not is_instance_valid(it["node"]):
			if is_instance_valid(it["node"]):
				it["node"].queue_free()
			continue
		if it["kind"] == "num":
			var n = it["node"]
			n.rect_position.y -= 55.0 * delta
			n.modulate.a = 1.0 - it["t"] / it["life"]
		keep.append(it)
	_ef_items = keep
	if _ef_on:
		_ef_watch()


func _ef_watch():
	var scn = get_tree().current_scene
	if scn == null:
		_ef_state = {}
		return
	if scn.has_node("cards_reward/HBoxContainer/reward_1"):
		if not _ef_reward_done:
			_ef_reward_done = true
			_ef_confetti()
		return
	_ef_reward_done = false
	if not scn.has_node("game_logic"):
		_ef_state = {}
		return
	_ef_lp("player", scn, "user_interface/top_info_box/player_info/lifepoints")
	_ef_lp("com", scn, "user_interface/top_info_box/com_info/lifepoints")
	_ef_field(scn, "player", "duel_field/player_side_zones/monster_")
	_ef_field(scn, "com", "duel_field/enemy_side_zones/monster_")


# Kilatan merah/oranye saat LP turun, kilatan hijau tipis saat LP naik, dan angka yang melayang.
func _ef_lp(side, scn, path):
	if not scn.has_node(path):
		return
	var lbl = scn.get_node(path)
	var v = int(lbl.text)
	var key = "lp_" + side
	var now = OS.get_ticks_msec()
	if not _ef_state.has(key):
		_ef_state[key] = {"base": v, "last": v, "ms": now, "moving": false}
		return
	var st = _ef_state[key]
	if v != st["last"]:
		if not st["moving"]:
			st["moving"] = true
			if v < st["base"]:
				if side == "player":
					_ef_flash_now(Color(1.0, 0.15, 0.1), 0.38)
				else:
					_ef_flash_now(Color(1.0, 0.65, 0.15), 0.32)
			else:
				_ef_flash_now(Color(0.3, 1.0, 0.4), 0.15)
		st["last"] = v
		st["ms"] = now
	elif st["moving"] and now - st["ms"] > 250:
		st["moving"] = false
		var diff = v - st["base"]
		if diff != 0:
			_ef_number(lbl, diff)
		st["base"] = v


func _ef_number(lbl, diff):
	var r = lbl.get_global_rect()
	var n = Label.new()
	n.text = ("+" if diff > 0 else "-") + str(abs(diff))
	n.rect_scale = Vector2(1.6, 1.6)
	n.rect_position = Vector2(r.position.x, r.position.y + r.size.y)
	n.mouse_filter = Control.MOUSE_FILTER_IGNORE
	if _badge_font != null:
		n.add_font_override("font", _badge_font)
	if diff > 0:
		n.add_color_override("font_color", Color(0.4, 1.0, 0.5, 1))
	else:
		n.add_color_override("font_color", Color(1.0, 0.35, 0.25, 1))
	_badge_layer.add_child(n)
	_ef_items.append({"kind": "num", "node": n, "t": 0.0, "life": 1.2})


# Api saat monster muncul, ledakan terang saat kartu berubah (fusion), bara saat monster hilang.
func _ef_field(scn, side, prefix):
	for i in range(5):
		var n = scn.get_node_or_null(prefix + str(i))
		if n == null:
			continue
		var key = side + "_slot_" + str(i)
		var vis = n.is_visible_in_tree()
		var cid = ""
		if vis:
			cid = str(n.this_card_id)
		if not _ef_state.has(key):
			_ef_state[key] = {"vis": vis, "id": cid}
			continue
		var st = _ef_state[key]
		if vis and not st["vis"]:
			_ef_burst_at(n, "summon")
		elif vis and st["vis"] and cid != "" and cid != st["id"]:
			_ef_burst_at(n, "fusion")
		elif not vis and st["vis"]:
			_ef_burst_at(n, "destroy")
		st["vis"] = vis
		st["id"] = cid


func _ef_burst_at(n, kind):
	var xf = n.get_global_transform_with_canvas()
	_ef_burst(xf.xform(n.rect_size * 0.5), kind)


func _ef_burst(pos, kind):
	var p = CPUParticles2D.new()
	p.position = pos
	p.one_shot = true
	p.local_coords = false
	p.explosiveness = 0.95
	p.emission_shape = CPUParticles2D.EMISSION_SHAPE_SPHERE
	p.emission_sphere_radius = 40.0
	p.direction = Vector2(0, -1)
	p.spread = 70.0
	p.gravity = Vector2(0, -260)
	p.initial_velocity = 140.0
	p.initial_velocity_random = 0.6
	p.scale_amount = 7.0
	p.scale_amount_random = 0.5
	p.amount = 36
	p.lifetime = 0.9
	var g = Gradient.new()
	g.set_color(0, Color(1.0, 0.95, 0.5, 1.0))
	g.set_color(1, Color(1.0, 0.15, 0.0, 0.0))
	g.add_point(0.45, Color(1.0, 0.55, 0.1, 0.9))
	if kind == "fusion":
		p.amount = 64
		p.lifetime = 1.1
		p.spread = 180.0
		p.initial_velocity = 210.0
		g.set_color(0, Color(1.0, 1.0, 0.9, 1.0))
		_ef_flash_now(Color(1.0, 1.0, 0.85), 0.45)
	elif kind == "destroy":
		p.amount = 28
		p.spread = 180.0
		p.gravity = Vector2(0, 140)
		g.set_color(0, Color(1.0, 0.5, 0.1, 1.0))
		g.set_color(1, Color(0.25, 0.1, 0.1, 0.0))
	p.color_ramp = g
	_badge_layer.add_child(p)
	p.emitting = true
	_ef_items.append({"kind": "p", "node": p, "t": 0.0, "life": p.lifetime + 0.4})


# Konfeti saat layar hadiah muncul setelah menang.
func _ef_confetti():
	var cols = [Color(1, 0.3, 0.3), Color(1, 0.8, 0.2), Color(0.3, 0.9, 0.4), Color(0.3, 0.7, 1), Color(0.9, 0.4, 1)]
	var vp = get_viewport().get_visible_rect().size
	for c in cols:
		var p = CPUParticles2D.new()
		p.position = Vector2(vp.x / 2, -10)
		p.one_shot = true
		p.local_coords = false
		p.explosiveness = 0.35
		p.emission_shape = CPUParticles2D.EMISSION_SHAPE_RECTANGLE
		p.emission_rect_extents = Vector2(vp.x / 2, 8)
		p.direction = Vector2(0, 1)
		p.spread = 35.0
		p.gravity = Vector2(0, 260)
		p.initial_velocity = 120.0
		p.initial_velocity_random = 0.5
		p.scale_amount = 8.0
		p.scale_amount_random = 0.4
		p.amount = 36
		p.lifetime = 3.2
		p.color = c
		_badge_layer.add_child(p)
		p.emitting = true
		_ef_items.append({"kind": "p", "node": p, "t": 0.0, "life": 3.8})
	_ef_flash_now(Color(1.0, 0.95, 0.7), 0.4)
