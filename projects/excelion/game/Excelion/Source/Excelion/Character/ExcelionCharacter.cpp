// Copyright Excelion. All Rights Reserved.

#include "Character/ExcelionCharacter.h"
#include "Data/ExcelionMechaDataAsset.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatComponent.h"
#include "Combat/SCoreComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputModifiers.h"
#include "Engine/LocalPlayer.h"

AExcelionCharacter::AExcelionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;

	// Follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Don't rotate character to camera direction by default
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;

	// Mesh default offset inside capsule
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// Components
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	SCoreComponent = CreateDefaultSubobject<USCoreComponent>(TEXT("SCoreComponent"));

	// Fallback visual mesh so character is immediately visible in Play without manual asset setup
	FallbackVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FallbackVisualMesh"));
	FallbackVisualMesh->SetupAttachment(RootComponent);
	FallbackVisualMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.8f));
	FallbackVisualMesh->SetVisibility(true, false);
	FallbackVisualMesh->SetHiddenInGame(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (DefaultCubeMesh.Succeeded())
	{
		FallbackVisualMesh->SetStaticMesh(DefaultCubeMesh.Object);
	}

	// CRITICAL: Input setup strategy
	// Option 1: Try to load editor-created IMC asset
	// Option 2: Fallback to runtime setup with proper key modifiers
	// NOTE: SetupPlayerInputComponent will handle axis input, this is for mapping context only
	
	// Try loading editor-created input assets (IMC + IA_* used by BP_ExcelionCharacter CDO)
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMCAsset(TEXT("/Game/Input/IMC_Default"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveAsset(TEXT("/Game/Input/IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> LookAsset(TEXT("/Game/Input/IA_Look"));
	static ConstructorHelpers::FObjectFinder<UInputAction> AttackAsset(TEXT("/Game/Input/IA_Attack"));
	static ConstructorHelpers::FObjectFinder<UInputAction> DashAsset(TEXT("/Game/Input/IA_Dash"));

	if (DefaultIMCAsset.Succeeded())
	{
		DefaultMappingContext = DefaultIMCAsset.Object;
		if (MoveAsset.Succeeded()) { MoveAction = MoveAsset.Object; }
		if (LookAsset.Succeeded()) { LookAction = LookAsset.Object; }
		if (AttackAsset.Succeeded()) { AttackAction = AttackAsset.Object; }
		if (DashAsset.Succeeded()) { DashAction = DashAsset.Object; }
		UE_LOG(LogTemp, Warning, TEXT("[INIT] Loaded editor input assets - IMC:%s Move:%s Look:%s Attack:%s Dash:%s"),
			DefaultMappingContext ? *DefaultMappingContext->GetName() : TEXT("NULL"),
			MoveAction ? *MoveAction->GetName() : TEXT("NULL"),
			LookAction ? *LookAction->GetName() : TEXT("NULL"),
			AttackAction ? *AttackAction->GetName() : TEXT("NULL"),
			DashAction ? *DashAction->GetName() : TEXT("NULL"));
		if (MoveAction && LookAction)
		{
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("[INIT] Editor IMC found but IA_Move/IA_Look missing - falling back to runtime input setup"));
	}
	
	// Fallback: Create runtime mapping context (but keep it simple)
	DefaultMappingContext = NewObject<UInputMappingContext>(this, TEXT("DefaultMappingContext"));
	if (!DefaultMappingContext)
	{
		UE_LOG(LogTemp, Error, TEXT("[INIT] ✗ FAILED to create runtime DefaultMappingContext!"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[INIT] ✓ Created runtime DefaultMappingContext"));

	// For runtime setup, create lightweight action references
	// These will be bound in SetupPlayerInputComponent via axis mappings
	MoveForwardAction = NewObject<UInputAction>(this, TEXT("MoveForwardAction"));
	if (MoveForwardAction)
	{
		MoveForwardAction->ValueType = EInputActionValueType::Axis1D;
		FEnhancedActionKeyMapping& WMapping = DefaultMappingContext->MapKey(MoveForwardAction, EKeys::W);
		FEnhancedActionKeyMapping& SMapping = DefaultMappingContext->MapKey(MoveForwardAction, EKeys::S);
		SMapping.Modifiers.Add(NewObject<UInputModifierNegate>());
	}

	MoveRightAction = NewObject<UInputAction>(this, TEXT("MoveRightAction"));
	if (MoveRightAction)
	{
		MoveRightAction->ValueType = EInputActionValueType::Axis1D;
		FEnhancedActionKeyMapping& DMapping = DefaultMappingContext->MapKey(MoveRightAction, EKeys::D);
		FEnhancedActionKeyMapping& AMapping = DefaultMappingContext->MapKey(MoveRightAction, EKeys::A);
		AMapping.Modifiers.Add(NewObject<UInputModifierNegate>());
	}

	LookXAction = NewObject<UInputAction>(this, TEXT("LookXAction"));
	if (LookXAction)
	{
		LookXAction->ValueType = EInputActionValueType::Axis1D;
		DefaultMappingContext->MapKey(LookXAction, EKeys::MouseX);
	}

	LookYAction = NewObject<UInputAction>(this, TEXT("LookYAction"));
	if (LookYAction)
	{
		LookYAction->ValueType = EInputActionValueType::Axis1D;
		FEnhancedActionKeyMapping& YMapping = DefaultMappingContext->MapKey(LookYAction, EKeys::MouseY);
		YMapping.Modifiers.Add(NewObject<UInputModifierNegate>());
	}

	AttackAction = NewObject<UInputAction>(this, TEXT("AttackAction"));
	if (AttackAction)
	{
		AttackAction->ValueType = EInputActionValueType::Boolean;
		DefaultMappingContext->MapKey(AttackAction, EKeys::LeftMouseButton);
	}

	DashAction = NewObject<UInputAction>(this, TEXT("DashAction"));
	if (DashAction)
	{
		DashAction->ValueType = EInputActionValueType::Boolean;
		DefaultMappingContext->MapKey(DashAction, EKeys::LeftShift);
	}

	UE_LOG(LogTemp, Warning, TEXT("[INIT] ✓ Runtime input actions created (Mappings: %d)"), 
		DefaultMappingContext->GetMappings().Num());
}

void AExcelionCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UE_LOG(LogTemp, Warning, TEXT("========== [EXCELION CHARACTER] PostInitializeComponents =========="));
	UE_LOG(LogTemp, Warning, TEXT("[AXION PIE DEBUG] Character Spawned - Name: %s, Location: %s"), *GetName(), *GetActorLocation().ToString());
	UE_LOG(LogTemp, Warning, TEXT("[AXION PIE DEBUG] GetMesh Valid: %d, FallbackVisualMesh Valid: %d"), 
		GetMesh() != nullptr, FallbackVisualMesh != nullptr);
	
	if (FallbackVisualMesh)
	{
		FallbackVisualMesh->SetVisibility(true, false);
		FallbackVisualMesh->SetHiddenInGame(false);
		UE_LOG(LogTemp, Warning, TEXT("[AXION PIE DEBUG] FallbackMesh visible forced"));
	}
	UE_LOG(LogTemp, Warning, TEXT("========== [EXCELION CHARACTER] PostInitializeComponents END =========="));

	ApplyMechaDataAsset();
}

void AExcelionCharacter::ApplyMechaDataAsset(UExcelionMechaDataAsset* InMechaDataAsset)
{
	UExcelionMechaDataAsset* TargetData = InMechaDataAsset ? InMechaDataAsset : MechaDataAsset;

	if (!TargetData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AExcelionCharacter] MechaDataAsset is NULL on %s! Runtime stats not set from SSOT."), *GetName());
		return;
	}

	const FExcelionMechaBaseStats& Stats = TargetData->BaseStats;

	if (HealthComponent)
	{
		HealthComponent->MaxHealth = Stats.MaxHP;
		HealthComponent->ResetHealth();
	}

	if (CombatComponent)
	{
		CombatComponent->AttackDamage = Stats.AttackPower;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
	}

	UE_LOG(LogTemp, Log, TEXT("[AExcelionCharacter] Applied MechaDataAsset (%s) to %s: MaxHP=%.1f, AttackPower=%.1f, MoveSpeed=%.1f"),
		*TargetData->GetName(), *GetName(), Stats.MaxHP, Stats.AttackPower, Stats.MoveSpeed);
}

void AExcelionCharacter::BeginPlay()
{
	UE_LOG(LogTemp, Error, TEXT("========== [CRITICAL] AExcelionCharacter::BeginPlay STARTED =========="));
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[INIT] BeginPlay - Name: %s, Controller: %s"), 
		*GetName(), Controller ? *Controller->GetName() : TEXT("NULL"));

	// ===== Movement Component Verification =====
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MOVE] CharacterMovement initialized - MaxWalkSpeed=%.1f, MovementMode=%d"), 
			MovementComp->MaxWalkSpeed, (int32)MovementComp->MovementMode);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MOVE] ERROR: CharacterMovement component is NULL!"));
	}
	
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AExcelionCharacter::OnDeath);
	}

	// Note: Enhanced Input Mapping Context registration.
	// CRITICAL: We MUST remove all existing contexts first, otherwise IMC_Default will override our runtime context.
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		UE_LOG(LogTemp, Warning, TEXT("[INIT] PlayerController found - clearing old contexts and registering new one"));
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			UE_LOG(LogTemp, Warning, TEXT("[INPUT] Cleared all existing mapping contexts"));

			const int32 MappingCount = DefaultMappingContext ? DefaultMappingContext->GetMappings().Num() : 0;
			if (DefaultMappingContext && MappingCount > 0)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemp, Warning, TEXT("[INPUT] DefaultMappingContext registered (%d mappings)"), MappingCount);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[INPUT] DefaultMappingContext skipped (NULL or 0 mappings) — legacy axis fallback active"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[INPUT] ERROR: EnhancedInputLocalPlayerSubsystem not found!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[INIT] ERROR: Controller is NULL or not PlayerController!"));
	}
	
	UE_LOG(LogTemp, Error, TEXT("========== [CRITICAL] AExcelionCharacter::BeginPlay END =========="));
}

void AExcelionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDashing)
	{
		UpdateDash(DeltaTime);
	}

	if (bIsInvulnerable)
	{
		InvulnTimer -= DeltaTime;
		if (InvulnTimer <= 0.f)
		{
			bIsInvulnerable = false;
		}
	}

	if (DashCooldownTimer > 0.f)
	{
		DashCooldownTimer -= DeltaTime;
	}
}

void AExcelionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UE_LOG(LogTemp, Error, TEXT("========== [CRITICAL] SetupPlayerInputComponent CALLED =========="));
	
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[CRITICAL] ERROR: PlayerInputComponent is NULL!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[INPUT] SetupPlayerInputComponent called - InputComponent valid"));

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		bool bBoundEnhancedInput = false;

		// Primary path: editor assets IA_Move / IA_Look (Axis2D) on BP_ExcelionCharacter CDO
		if (MoveAction && LookAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AExcelionCharacter::Move);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AExcelionCharacter::Look);
			UE_LOG(LogTemp, Warning, TEXT("[INPUT] Bound IA_Move / IA_Look (Axis2D editor assets)"));
			bBoundEnhancedInput = true;
		}
		else if (MoveForwardAction && MoveRightAction && LookXAction && LookYAction)
		{
			EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AExcelionCharacter::MoveForward);
			EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AExcelionCharacter::MoveRight);
			EnhancedInputComponent->BindAction(LookXAction, ETriggerEvent::Triggered, this, &AExcelionCharacter::LookX);
			EnhancedInputComponent->BindAction(LookYAction, ETriggerEvent::Triggered, this, &AExcelionCharacter::LookY);
			UE_LOG(LogTemp, Warning, TEXT("[INPUT] Bound runtime Axis1D move/look actions"));
			bBoundEnhancedInput = true;
		}

		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AExcelionCharacter::Attack);
			UE_LOG(LogTemp, Warning, TEXT("[INPUT] Bound AttackAction"));
		}
		if (DashAction)
		{
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AExcelionCharacter::Dash);
			UE_LOG(LogTemp, Warning, TEXT("[INPUT] Bound DashAction"));
		}

		// Always bind legacy axis — works when IMC is empty or Enhanced Input fails
		PlayerInputComponent->BindAxis("MoveForward", this, &AExcelionCharacter::MoveForwardAxis);
		PlayerInputComponent->BindAxis("MoveRight", this, &AExcelionCharacter::MoveRightAxis);
		PlayerInputComponent->BindAxis("LookUp", this, &AExcelionCharacter::LookUpAxis);
		PlayerInputComponent->BindAxis("Turn", this, &AExcelionCharacter::TurnAxis);
		UE_LOG(LogTemp, Warning, TEXT("[INPUT] Legacy axis fallback ALWAYS bound (MoveForward/MoveRight/Turn/LookUp)"));

		if (!bBoundEnhancedInput)
		{
			UE_LOG(LogTemp, Warning, TEXT("[INPUT] Enhanced move/look not bound — relying on DefaultInput.ini axis mappings"));
		}
	}
	else
	{
		// FALLBACK: Not Enhanced Input - use basic axis bindings
		UE_LOG(LogTemp, Warning, TEXT("[INPUT] ! Not EnhancedInputComponent - using DefaultInput.ini axis bindings"));
		PlayerInputComponent->BindAxis("MoveForward", this, &AExcelionCharacter::MoveForwardAxis);
		PlayerInputComponent->BindAxis("MoveRight", this, &AExcelionCharacter::MoveRightAxis);
		PlayerInputComponent->BindAxis("LookUp", this, &AExcelionCharacter::LookUpAxis);
		PlayerInputComponent->BindAxis("Turn", this, &AExcelionCharacter::TurnAxis);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[INPUT] SetupPlayerInputComponent END"));
}

void AExcelionCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (bIsDashing || IsDead())
	{
		return;
	}

	if (!MovementVector.IsNearlyZero())
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AExcelionCharacter::MoveForward(const FInputActionValue& Value)
{
	const float ForwardValue = Value.Get<float>();
	const FVector CurrentLoc = GetActorLocation();
	const FVector CurrentVel = GetCharacterMovement()->Velocity;
	
	UE_LOG(LogTemp, Error, TEXT("[WASD-FORWARD] Input=%.2f | Pos=(%.0f,%.0f,%.0f) | Vel=(%.0f,%.0f,%.0f) | MovMode=%d"), 
		ForwardValue, CurrentLoc.X, CurrentLoc.Y, CurrentLoc.Z, CurrentVel.X, CurrentVel.Y, CurrentVel.Z, 
		(int32)GetCharacterMovement()->MovementMode);
	
	if (bIsDashing || IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MOVE] MoveForward blocked: bIsDashing=%d, IsDead=%d"), bIsDashing, IsDead());
		return;
	}
	
	if (ForwardValue != 0.f)
	{
		const FVector ForwardDir = GetActorForwardVector();
		UE_LOG(LogTemp, Error, TEXT("[WASD-FORWARD-APPLY] Adding movement: Value=%.2f, Direction=(%.2f,%.2f,%.2f)"), 
			ForwardValue, ForwardDir.X, ForwardDir.Y, ForwardDir.Z);
		AddMovementInput(ForwardDir, ForwardValue);
	}
}

void AExcelionCharacter::MoveRight(const FInputActionValue& Value)
{
	const float RightValue = Value.Get<float>();
	const FVector CurrentLoc = GetActorLocation();
	const FVector CurrentVel = GetCharacterMovement()->Velocity;
	
	UE_LOG(LogTemp, Error, TEXT("[WASD-RIGHT] Input=%.2f | Pos=(%.0f,%.0f,%.0f) | Vel=(%.0f,%.0f,%.0f) | MovMode=%d"), 
		RightValue, CurrentLoc.X, CurrentLoc.Y, CurrentLoc.Z, CurrentVel.X, CurrentVel.Y, CurrentVel.Z, 
		(int32)GetCharacterMovement()->MovementMode);
	
	if (bIsDashing || IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MOVE] MoveRight blocked: bIsDashing=%d, IsDead=%d"), bIsDashing, IsDead());
		return;
	}
	
	if (RightValue != 0.f)
	{
		const FVector RightDir = GetActorRightVector();
		UE_LOG(LogTemp, Error, TEXT("[WASD-RIGHT-APPLY] Adding movement: Value=%.2f, Direction=(%.2f,%.2f,%.2f)"), 
			RightValue, RightDir.X, RightDir.Y, RightDir.Z);
		AddMovementInput(RightDir, RightValue);
	}
}

void AExcelionCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void AExcelionCharacter::LookX(const FInputActionValue& Value)
{
	float X = Value.Get<float>();
	if (X != 0.f)
	{
		AddControllerYawInput(X);
	}
}

void AExcelionCharacter::LookY(const FInputActionValue& Value)
{
	float Y = Value.Get<float>();
	if (Y != 0.f)
	{
		AddControllerPitchInput(Y);
	}
}

// Fallback axis input handlers (for basic DefaultInput.ini axis mappings)
void AExcelionCharacter::MoveForwardAxis(float Value)
{
	if (Value != 0.f && !bIsDashing && !IsDead())
	{
		AddMovementInput(GetActorForwardVector(), Value);
		UE_LOG(LogTemp, Warning, TEXT("[MOVE-AXIS] Forward: %.2f"), Value);
	}
}

void AExcelionCharacter::MoveRightAxis(float Value)
{
	if (Value != 0.f && !bIsDashing && !IsDead())
	{
		AddMovementInput(GetActorRightVector(), Value);
		UE_LOG(LogTemp, Warning, TEXT("[MOVE-AXIS] Right: %.2f"), Value);
	}
}

void AExcelionCharacter::LookUpAxis(float Value)
{
	if (Value != 0.f)
	{
		AddControllerPitchInput(Value);
		UE_LOG(LogTemp, Warning, TEXT("[LOOK-AXIS] Up: %.2f"), Value);
	}
}

void AExcelionCharacter::TurnAxis(float Value)
{
	if (Value != 0.f)
	{
		AddControllerYawInput(Value);
		UE_LOG(LogTemp, Warning, TEXT("[LOOK-AXIS] Turn: %.2f"), Value);
	}
}

void AExcelionCharacter::Attack(const FInputActionValue& Value)
{
	if (IsDead() || bIsDashing)
	{
		return;
	}

	if (CombatComponent)
	{
		CombatComponent->TryAttack();
	}
}

void AExcelionCharacter::Dash(const FInputActionValue& Value)
{
	if (IsDead() || bIsDashing || DashCooldownTimer > 0.f)
	{
		return;
	}
	StartDash();
}

void AExcelionCharacter::StartDash()
{
	bIsDashing = true;
	bIsInvulnerable = true;
	DashTimer = DashDuration;
	InvulnTimer = InvulnerabilityDuration;
	DashCooldownTimer = DashCooldown;

	// Dash direction: input direction or forward
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	if (Velocity.SizeSquared() > 10.f)
	{
		DashDirection = Velocity.GetSafeNormal();
	}
	else
	{
		DashDirection = GetActorForwardVector();
	}

	// Disable movement during dash
	GetCharacterMovement()->DisableMovement();
}

void AExcelionCharacter::UpdateDash(float DeltaTime)
{
	DashTimer -= DeltaTime;

	const float Alpha = 1.f - (DashTimer / DashDuration);
	const FVector DashOffset = DashDirection * (DashDistance * DeltaTime / DashDuration);
	AddActorWorldOffset(DashOffset, true);

	if (DashTimer <= 0.f)
	{
		EndDash();
	}
}

void AExcelionCharacter::EndDash()
{
	bIsDashing = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

float AExcelionCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsInvulnerable || IsDead())
	{
		return 0.f;
	}

	if (HealthComponent)
	{
		return HealthComponent->ApplyDamage(DamageAmount);
	}
	return 0.f;
}

bool AExcelionCharacter::IsDead() const
{
	return HealthComponent && HealthComponent->IsDead();
}

void AExcelionCharacter::OnDeath()
{
	// Disable input and movement on death
	GetCharacterMovement()->DisableMovement();
	DisableInput(Cast<APlayerController>(Controller));

	// TODO: Notify GameMode for Defeat state (Phase 6)
}
