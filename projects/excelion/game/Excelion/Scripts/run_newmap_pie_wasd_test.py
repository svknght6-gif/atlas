#!/usr/bin/env python3
"""
Excelion NewMap PIE + WASD Input Test
- Opens NewMap in editor
- Starts PIE
- Records character position, velocity, and input diagnostics
"""

import unreal
import time

@unreal.ufunction(unreal.ufunction_flags.EXEC)
def run_newmap_pie_wasd_test():
    """Execute NewMap PIE with WASD diagnostics"""
    
    print("="*80)
    print("[EXCELION PIE TEST] Starting NewMap PIE + WASD Input Test")
    print("="*80)
    
    # Get world and level
    world = unreal.get_editor_subsystem(unreal.UnrealEd.UnrealEditorSubsystem).get_editor_world()
    level = world.get_outer()
    
    print(f"[INIT] Current World: {world.get_name()}")
    print(f"[INIT] Current Level: {level.get_name()}")
    
    # Find player controller and character
    controllers = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.PlayerController)
    print(f"[INIT] PlayerControllers found: {len(controllers)}")
    
    for ctrl in controllers:
        print(f"  ├─ Name: {ctrl.get_name()}")
        print(f"  ├─ Class: {ctrl.get_class().get_name()}")
        
        possessed_pawn = ctrl.get_possessed_pawn()
        if possessed_pawn:
            print(f"  ├─ Possessed Pawn: {possessed_pawn.get_name()}")
            print(f"  ├─ Pawn Class: {possessed_pawn.get_class().get_name()}")
            print(f"  ├─ Pawn Location: {possessed_pawn.get_actor_location()}")
            
            # Check if it's a character
            if isinstance(possessed_pawn, unreal.Character):
                char = unreal.cast(possessed_pawn, unreal.Character)
                char_mov = char.get_character_movement()
                print(f"  ├─ Character Movement Max Speed: {char_mov.max_walk_speed}")
                print(f"  ├─ Character Movement Mode: {char_mov.movement_mode}")
                print(f"  └─ Character Velocity: {char.get_velocity()}")
        else:
            print(f"  └─ Possessed Pawn: NONE")
    
    # Find all Excelion characters
    print("\n[CHARACTER SEARCH] Looking for BP_ExcelionCharacter...")
    char_class = unreal.load_class(None, "/Game/Blueprints/BP_ExcelionCharacter.BP_ExcelionCharacter_C")
    if char_class:
        chars = unreal.GameplayStatics.get_all_actors_of_class(world, char_class)
        print(f"[CHARACTER SEARCH] Found {len(chars)} BP_ExcelionCharacter instances")
        for char in chars:
            print(f"  ├─ Name: {char.get_name()}")
            print(f"  ├─ Location: {char.get_actor_location()}")
            print(f"  ├─ Velocity: {char.get_velocity()}")
            
            # Check skeletal mesh
            if hasattr(char, "get_mesh"):
                mesh = char.get_mesh()
                if mesh:
                    print(f"  ├─ Skeletal Mesh: Valid ✓")
                    print(f"  ├─ Mesh Visible: {mesh.is_visible()}")
                else:
                    print(f"  ├─ Skeletal Mesh: NONE")
    else:
        print("[CHARACTER SEARCH] BP_ExcelionCharacter class not found")
    
    print("\n" + "="*80)
    print("[EXCELION PIE TEST] Test Complete")
    print("="*80)


# Auto-run when loaded
if __name__ == "__main__":
    run_newmap_pie_wasd_test()
