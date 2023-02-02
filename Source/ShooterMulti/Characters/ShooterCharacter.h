#pragma once

#include "HealthCharacter.h"
#include "../Weapons/WeaponComponent.h"
#include "PlayerCameraComponent.h"
#include "ShooterCharacter.generated.h"

USTRUCT()
struct FSavedMove
{
	GENERATED_USTRUCT_BODY();

	float deltaTime = 0.f;
	FVector moveDirection = FVector();
	FVector pawnPosition = FVector();
};

UENUM(BlueprintType)
enum class EShooterCharacterState : uint8
{
	IdleRun,
	Aim,
	Sprint,
	Reload,
	Jump,
	Falling,
	Punch,
	Dead,
	PushButton
};

UCLASS()
class SHOOTERMULTI_API AShooterCharacter : public AHealthCharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly, Replicated, VisibleDefaultsOnly, Category = "Character|Shooter")
	UWeaponComponent* Weapon;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Character|Shooter")
	UPlayerCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Character|Shooter")
	EShooterCharacterState State;
	EShooterCharacterState PrevState;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float AimPitch;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float AimYaw;


	void UpdateShooterRotation();

	UFUNCTION(Server, Reliable)
	void AimOffsetsOnServer(float Pitch, float Yaw);

	void PlayPushButtonAnim();

	void PlayPunchAnim();

	void Falling() override;

	void BeginPlay() override;

	void Invincibility(float Duration);

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Shooter")
	void InvincibilityFX(float Duration);
	void InvincibilityFX_Implementation(float Duration) {};

	void UpdateShooterWeapon();

	bool Shoot();
	UFUNCTION(Server, Reliable)
	void PlayShotFXOnServer(FShoot shoot);
	UFUNCTION(NetMulticast, Reliable)
	void PlayShotFXMulticast(FShoot shoot);

	UFUNCTION(Server, Reliable)
	void ReloadOnServer();
	UFUNCTION(NetMulticast, Reliable)
	void ReloadMulticast();

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	float movementMultiplyer = 1.f;

	bool bIsShooting = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float SprintSpeed = 1000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float AimWalkSpeed = 180.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float ReloadWalkSpeed = 200.f;

	UPROPERTY(BlueprintReadOnly)
	float RunSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MinSprintMagnitude = .3f;

	AShooterCharacter();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	EShooterCharacterState GetState() const;
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void SetState(EShooterCharacterState InState);

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	UWeaponComponent* GetWeaponComponent();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	UPlayerCameraComponent* GetCameraComponent();

	void InitPlayer();

	void OnRep_Team() override;

	UFUNCTION(Server, Reliable)
	void InitTeamColorOnServer(ETeam InTeam);

	void Tick(float DeltaTime) override;

	UFUNCTION()
	void StartSprint();
	UFUNCTION()
	void EndSprint();

	UFUNCTION()
	void StartJump();
	UFUNCTION()
	void EndJump();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartAim(bool bStart=true);
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void EndAim(bool bEnd=true);

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartShoot();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void EndShoot();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartReload();
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Character|Shooter")
	void EndReloadOnServer();
	UFUNCTION(NetMulticast, Reliable)
	void EndReloadMulticast();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void AbortReload();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Character|Shooter")
	void PushButtonOnServer();
	UFUNCTION(NetMulticast, Reliable)
	void PushButtonMulticast();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void InflictPushButton();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Character|Shooter")
	void StartAimOnServer();
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Character|Shooter")
	void EndAimOnServer();

	UFUNCTION(Server, Reliable)
	void StartSprintOnServer();

	UFUNCTION(Server, Reliable)
	void EndSprintOnServer();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Character|Shooter")
	void PunchOnServer();
	UFUNCTION(NetMulticast, Reliable)
	void PunchMulticast();

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Shooter")
	void RefreshTeamHUD(ETeam InTeam);
	void RefreshTeamHUD_Implementation(ETeam InTeam) {};

	void StartDisappearMulticast_Implementation() override;
	void FinishDisappear() override;

	UFUNCTION(Server, Reliable)
	void ResetEffectPickupOnServer();

	float GetStateSpeed() const;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};