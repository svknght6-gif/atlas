// Copyright Excelion. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "ExcelionCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UHealthComponent;
class UCombatComponent;
class USCoreComponent;
class UExcelionMechaDataAsset;

/**
 * AXION Player Character — Prototype v0.1
 * Movement, Camera, Attack, Dash, Health, S-Core.
 */
UCLASS()
class EXCELION_API AExcelionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AExcelionCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	/** Applies stats from MechaDataAsset SSOT to runtime components. */
	UFUNCTION(BlueprintCallable, Category = "AXION|Mecha")
	void ApplyMechaDataAsset(UExcelionMechaDataAsset* InMechaDataAsset = nullptr);

	/** Called when character dies. */
	UFUNCTION()
	void OnDeath();

	UFUNCTION(BlueprintPure, Category = "AXION|Health")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "AXION|Dash")
	bool IsDashing() const { return bIsDashing; }

	UFUNCTION(BlueprintPure, Category = "AXION|Dash")
	bool IsInvulnerable() const { return bIsInvulnerable; }

	UFUNCTION(BlueprintPure, Category = "AXION|Combat")
	USCoreComponent* GetSCoreComponent() const { return SCoreComponent; }

protected:
	// ----- Camera -----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	// ----- Components -----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USCoreComponent* SCoreComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mecha")
	UExcelionMechaDataAsset* MechaDataAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	UStaticMeshComponent* FallbackVisualMesh;

	// ----- Enhanced Input (references to existing Input assets) -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Default Mapping Context"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	// Editor assets: IA_Move / IA_Look (Axis2D) — wired on BP_ExcelionCharacter CDO
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Move Action"))
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Look Action"))
	TObjectPtr<class UInputAction> LookAction;

	// Movement actions - separate for each axis (Axis1D for proper value handling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Move Forward Action"))
	TObjectPtr<class UInputAction> MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Move Right Action"))
	TObjectPtr<class UInputAction> MoveRightAction;

	// Look actions - separate for each axis (Axis1D)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Look X Action"))
	TObjectPtr<class UInputAction> LookXAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Look Y Action"))
	TObjectPtr<class UInputAction> LookYAction;

	// Combat actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Attack Action"))
	TObjectPtr<class UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "Dash Action"))
	TObjectPtr<class UInputAction> DashAction;

	// ----- Dash -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDistance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDuration = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float InvulnerabilityDuration = 0.25f;

	bool bIsDashing = false;
	bool bIsInvulnerable = false;
	float DashTimer = 0.f;
	float InvulnTimer = 0.f;
	float DashCooldownTimer = 0.f;
	FVector DashDirection = FVector::ZeroVector;

	// ----- Input Handlers -----
	void Move(const FInputActionValue& Value);
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void LookX(const FInputActionValue& Value);
	void LookY(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);
	void Dash(const FInputActionValue& Value);

	// Fallback axis input handlers (for basic DefaultInput.ini bindings)
	void MoveForwardAxis(float Value);
	void MoveRightAxis(float Value);
	void LookUpAxis(float Value);
	void TurnAxis(float Value);

	void StartDash();
	void UpdateDash(float DeltaTime);
	void EndDash();

	/** Override damage reception to respect invulnerability. */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
};
