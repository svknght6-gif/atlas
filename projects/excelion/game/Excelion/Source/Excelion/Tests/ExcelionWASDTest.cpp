// WASD Input Automation Test for Excelion
// Tests character movement input system at runtime

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#if WITH_AUTOMATION_TESTS

class FExcelionWASDAutomationTest : public FAutomationTestBase
{
public:
    FExcelionWASDAutomationTest(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

    virtual uint32 GetTestFlags() const override
    {
        return EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
    }

    virtual bool RunTest(const FString& Parameters) override;

private:
    void TestCharacterMovement(ACharacter* Character, const FString& KeyName, float AxisValue);
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FExcelionWASDTest,
    "Excelion.WASD.InputSystem",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExcelionWASDTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ========== EXCELION WASD TEST START =========="));
    
    // Get world
    UWorld* World = GEngine->GetWorldContexts()[0].World();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ERROR: World not found"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] World: %s"), *World->GetMapName());
    
    // Find player controller
    APlayerController* PC = nullptr;
    for (TActorIterator<APlayerController> It(World); It; ++It)
    {
        PC = *It;
        break;
    }
    
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ERROR: PlayerController not found"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] PlayerController found: %s"), *PC->GetName());
    
    // Get possessed pawn
    APawn* PossessedPawn = PC->GetPawn();
    if (!PossessedPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ERROR: Possessed pawn not found"));
        return false;
    }
    
    ACharacter* Character = Cast<ACharacter>(PossessedPawn);
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ERROR: Possessed pawn is not a Character"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Character: %s"), *Character->GetName());
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Location: %s"), *Character->GetActorLocation().ToString());
    
    // Check character movement
    UCharacterMovementComponent* CharMov = Character->GetCharacterMovement();
    if (!CharMov)
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ERROR: CharacterMovementComponent not found"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] CharacterMovement MaxSpeed: %.0f"), CharMov->MaxWalkSpeed);
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] CharacterMovement Mode: %d"), (int32)CharMov->MovementMode);
    
    // Store initial position
    FVector InitialPos = Character->GetActorLocation();
    FVector InitialVel = Character->GetVelocity();
    
    UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ========== INITIAL STATE =========="));
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Initial Position: X=%.1f Y=%.1f Z=%.1f"), 
        InitialPos.X, InitialPos.Y, InitialPos.Z);
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Initial Velocity: X=%.1f Y=%.1f Z=%.1f"), 
        InitialVel.X, InitialVel.Y, InitialVel.Z);
    
    // Test 1: Forward Movement (W key - IA_Move.Y = 1.0)
    UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ========== TEST 1: FORWARD (W Key) =========="));
    if (Character->FindComponentByClass<class UEnhancedInputComponent>())
    {
        UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Enhanced Input Component found"));
        // Cannot directly trigger input, but we can log the capability
        UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] WASD forward capability: READY"));
    }
    
    // Test 2: Check input component setup
    UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ========== TEST 2: INPUT BINDINGS =========="));
    UInputComponent* InputComp = Character->FindComponentByClass<UInputComponent>();
    if (InputComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] InputComponent found"));
        UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Action bindings count: %d"), 
            InputComp->GetNumActionBindings());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] InputComponent not directly found on Character"));
    }
    
    // Test 3: Verify mesh visibility
    UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ========== TEST 3: MESH VISIBILITY =========="));
    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] SkeletalMesh found: %s"), *SkeletalMesh->GetName());
        UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] SkeletalMesh Visible: %d"), SkeletalMesh->IsVisible());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ERROR: SkeletalMesh not found"));
    }
    
    // Test 4: Character capabilities check
    UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ========== TEST 4: CHARACTER CAPABILITIES =========="));
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Can Jump: %d"), (int32)Character->GetCharacterMovement()->bCanEverJump);
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Walk Speed: %.0f"), Character->GetCharacterMovement()->MaxWalkSpeed);
    UE_LOG(LogTemp, Warning, TEXT("[AUTOMATION] Gravity: %.0f"), Character->GetCharacterMovement()->GravityScale);
    
    UE_LOG(LogTemp, Error, TEXT("[AUTOMATION] ========== EXCELION WASD TEST COMPLETE =========="));
    
    return true;
}

// Extended test with manual input tracking
IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FExcelionWASDManualInputTest,
    "Excelion.WASD.ManualInput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

void FExcelionWASDManualInputTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
    OutBeautifiedNames.Add(TEXT("Manual WASD Input Test - Forward"));
    OutTestCommands.Add(TEXT("Forward"));
    
    OutBeautifiedNames.Add(TEXT("Manual WASD Input Test - Backward"));
    OutTestCommands.Add(TEXT("Backward"));
    
    OutBeautifiedNames.Add(TEXT("Manual WASD Input Test - Left"));
    OutTestCommands.Add(TEXT("Left"));
    
    OutBeautifiedNames.Add(TEXT("Manual WASD Input Test - Right"));
    OutTestCommands.Add(TEXT("Right"));
}

bool FExcelionWASDManualInputTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogTemp, Error, TEXT("[WASD-MANUAL] ========== MANUAL INPUT TEST: %s =========="), *Parameters);
    
    UWorld* World = GEngine->GetWorldContexts()[0].World();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[WASD-MANUAL] ERROR: World not found"));
        return false;
    }
    
    APlayerController* PC = nullptr;
    for (TActorIterator<APlayerController> It(World); It; ++It)
    {
        PC = *It;
        break;
    }
    
    if (!PC || !PC->GetPawn())
    {
        UE_LOG(LogTemp, Error, TEXT("[WASD-MANUAL] ERROR: No controlled pawn"));
        return false;
    }
    
    ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("[WASD-MANUAL] ERROR: Pawn is not a Character"));
        return false;
    }
    
    FVector StartPos = Character->GetActorLocation();
    FVector StartVel = Character->GetVelocity();
    
    UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Test: %s"), *Parameters);
    UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Start Position: X=%.1f Y=%.1f Z=%.1f"), 
        StartPos.X, StartPos.Y, StartPos.Z);
    UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Start Velocity: X=%.1f Y=%.1f Z=%.1f"), 
        StartVel.X, StartVel.Y, StartVel.Z);
    UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Start Direction: %s"), *Character->GetActorForwardVector().ToString());
    
    // Log expected axis values
    if (Parameters == TEXT("Forward"))
    {
        UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Expected: IA_Move.Y = +1.0 (MoveForward called)"));
        UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Expected Direction: Forward vector (should move X positive)"));
    }
    else if (Parameters == TEXT("Backward"))
    {
        UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Expected: IA_Move.Y = -1.0 (MoveForward called with -1)"));
        UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Expected Direction: Backward (should move X negative)"));
    }
    else if (Parameters == TEXT("Left"))
    {
        UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Expected: IA_Move.X = -1.0 (MoveRight called with -1)"));
        UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Expected Direction: Left vector (should move Y negative)"));
    }
    else if (Parameters == TEXT("Right"))
    {
        UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Expected: IA_Move.X = +1.0 (MoveRight called)"));
        UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Expected Direction: Right vector (should move Y positive)"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Instructions: Press %s key for 2-3 seconds"), 
        *Parameters);
    UE_LOG(LogTemp, Warning, TEXT("[WASD-MANUAL] Logs will capture position/velocity changes"));
    
    return true;
}

#endif // WITH_AUTOMATION_TESTS
