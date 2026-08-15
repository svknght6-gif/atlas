# A1 Input Diagnostic + Fix — IMC mappings, BP CDO wiring, GameMode pawn
import unreal

def diagnose_and_fix_input():
    unreal.log("=== A1 Input Diagnostic + Fix Started ===")
    ok = True

    # 1. Load input assets
    imc = unreal.EditorAssetLibrary.load_asset("/Game/Input/IMC_Default")
    ia_move = unreal.EditorAssetLibrary.load_asset("/Game/Input/IA_Move")
    ia_look = unreal.EditorAssetLibrary.load_asset("/Game/Input/IA_Look")
    ia_attack = unreal.EditorAssetLibrary.load_asset("/Game/Input/IA_Attack")
    ia_dash = unreal.EditorAssetLibrary.load_asset("/Game/Input/IA_Dash")

    if not all([imc, ia_move, ia_look, ia_attack, ia_dash]):
        unreal.log_error("[A1-FAIL] Missing input assets under /Game/Input/")
        return False

    # 2. Report IMC mapping count BEFORE fix
    mappings = imc.get_editor_property("mappings")
    unreal.log(f"[A1-DIAG] IMC_Default mappings BEFORE fix: {len(mappings)}")

    if len(mappings) < 7:
        unreal.log_warning("[A1-FIX] IMC_Default under-mapped — running wire_imc_default_mappings")
        import wire_imc_default_mappings
        if not wire_imc_default_mappings.wire_and_verify_imc_default():
            unreal.log_error("[A1-FAIL] wire_imc_default_mappings failed")
            return False
        imc = unreal.EditorAssetLibrary.load_asset("/Game/Input/IMC_Default")
        mappings = imc.get_editor_property("mappings")
        unreal.log(f"[A1-DIAG] IMC_Default mappings AFTER wire: {len(mappings)}")

    # 3. Wire BP_ExcelionCharacter CDO
    bp_char_path = "/Game/Blueprints/BP_ExcelionCharacter"
    bp_char_class = unreal.load_object(None, f"{bp_char_path}.BP_ExcelionCharacter_C")
    if not bp_char_class:
        unreal.log_error("[A1-FAIL] BP_ExcelionCharacter not found")
        return False

    cdo = unreal.get_default_object(bp_char_class)
    cdo.set_editor_property("default_mapping_context", imc)
    cdo.set_editor_property("move_action", ia_move)
    cdo.set_editor_property("look_action", ia_look)
    cdo.set_editor_property("attack_action", ia_attack)
    cdo.set_editor_property("dash_action", ia_dash)
    unreal.EditorAssetLibrary.save_loaded_asset(unreal.EditorAssetLibrary.load_asset(bp_char_path))
    unreal.log("[A1-FIX] BP_ExcelionCharacter CDO input assets saved")

    # 4. Wire BP_ExcelionGameMode DefaultPawnClass -> BP_ExcelionCharacter
    bp_gm_path = "/Game/Blueprints/BP_ExcelionGameMode"
    bp_gm_class = unreal.load_object(None, f"{bp_gm_path}.BP_ExcelionGameMode_C")
    if bp_gm_class:
        cdo_gm = unreal.get_default_object(bp_gm_class)
        cdo_gm.set_editor_property("default_pawn_class", bp_char_class)
        unreal.EditorAssetLibrary.save_loaded_asset(unreal.EditorAssetLibrary.load_asset(bp_gm_path))
        unreal.log("[A1-FIX] BP_ExcelionGameMode DefaultPawnClass -> BP_ExcelionCharacter")
    else:
        unreal.log_warning("[A1-WARN] BP_ExcelionGameMode not found — skipping GameMode pawn fix")
        ok = False

    # 5. Read-back verification
    for prop, expected in [
        ("default_mapping_context", "IMC_Default"),
        ("move_action", "IA_Move"),
        ("look_action", "IA_Look"),
    ]:
        val = cdo.get_editor_property(prop)
        name = val.get_name() if val else "None"
        if name == expected:
            unreal.log(f"[A1-PASS] BP CDO {prop} = {name}")
        else:
            unreal.log_error(f"[A1-FAIL] BP CDO {prop} = {name} (expected {expected})")
            ok = False

    mappings = imc.get_editor_property("mappings")
    if len(mappings) >= 7:
        unreal.log(f"[A1-PASS] IMC_Default has {len(mappings)} mappings")
    else:
        unreal.log_error(f"[A1-FAIL] IMC_Default still has only {len(mappings)} mappings")
        ok = False

    unreal.log(f"=== A1 Input Diagnostic + Fix {'PASSED' if ok else 'FAILED'} ===")
    return ok

if __name__ == "__main__":
    diagnose_and_fix_input()
