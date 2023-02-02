#pragma once

#include "ObjectPool.h"
#include "Components/SkeletalMeshComponent.h"
#include "WeaponComponent.generated.h"

USTRUCT(/*BlueprintInternalUseOnly*/)
struct FLaserWeaponData
{
	GENERATED_BODY()

	UPROPERTY(/*BlueprintInternalUseOnly*/)
	FTransform LookTransform;
	UPROPERTY()
	FTransform MuzzleTransform;

	UPROPERTY(/*BlueprintInternalUseOnly*/)
	float Damages;
	UPROPERTY(/*BlueprintInternalUseOnly*/)
	float Knockback;
	UPROPERTY(/*BlueprintInternalUseOnly*/)
	float Spread = 0.f;
	UPROPERTY(/*BlueprintInternalUseOnly*/)
	float MaxDistance = 10000.f;
};

USTRUCT()
struct FRaycastInfo
{
	GENERATED_BODY()

	// has the raycast hit an object?
	UPROPERTY()
	bool bHit;
	// raycast start
	UPROPERTY()
	FVector LookLocation;
	// raycast direction
	UPROPERTY()
	FVector LookDirection;

	// Unreal raycast
	UPROPERTY()
	FHitResult HitResult;
	// if collision with non character
	UPROPERTY()
	bool bHitCharacter;
};

USTRUCT()
struct FShoot
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Shooter = nullptr;

	UPROPERTY()
	FRaycastInfo RaycastInfo;

	UPROPERTY()
	FLaserWeaponData WeaponData;
};

UCLASS()
class SHOOTERMULTI_API UWeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

private:
	// Weapon Utility.

	// apply damage or knock back on potentially hit object
	UFUNCTION(/*BlueprintCallable, Category = "Character|Shooter|Weapon"*/)
	void ApplyHit(FShoot shoot);
	UFUNCTION(Server, Reliable)
	void ApplyHitOnServer(FShoot shoot);

	FLaserWeaponData GetWeaponData() const;

	// raycast to detect collision
	FRaycastInfo ShootRaycast(AActor* Causer);

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter|Weapon")
	void MakeImpactDecal(	const FHitResult& FromHit,
							UMaterialInterface* ImpactDecalMaterial,
							float ImpactDecalSizeMin,
							float ImpactDecalSizeMax);

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter|Weapon")
	void MakeLaserBeam(	FVector Start,
						FVector End,
						UParticleSystem* BeamParticles,
						float InBeamIntensity,
						FLinearColor Color,
						UCurveFloat* InBeamIntensityCurve);

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter|Weapon")
	void MakeImpactParticles(UParticleSystem* ImpactParticles, const FHitResult& FromHit, float Scale = 1.f);

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	class UParticleSystem* ImpactParticle;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	class UParticleSystem* BeamParticle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	USoundBase* ShotSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	USoundBase* ShotEmptySound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	UMaterial* ImpactDecalMat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	class UCurveFloat* BeamIntensityCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	class UParticleSystem* MuzzleSmokeParticle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	TSubclassOf<class UMatineeCameraShake> ShootShake;

	float ShootTimer = 0.0f;

	ObjectPool<class ABeamLight> LightPool;

	void SpawnEmitterAtLocation(UParticleSystem* EmitterTemplate,
								const FTransform& SpawnTransform,
								const FVector& Source = FVector::ZeroVector,
								const FVector& Target = FVector::ZeroVector);

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon", meta = (ClampMin = "0"))
	int MaxAmmo = 50;

	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Character|Shooter|Weapon", meta = (ClampMin = "0"))
	int AmmoCount = 50;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Character|Shooter|Weapon")
	int LoadedAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Character|Shooter|Weapon", meta = (ClampMin = "0"))
	int WeaponMagazineSize = 20;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon", meta = (ClampMin = "0"))
	float FireRate = 0.24f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float ImpactDecalLifeSpan = 30.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float ImpactDecalSize = 10.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float BeamIntensity = 3000.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float WeaponDamage = 20.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float WeaponPunchDamage = 30.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float WeaponKnockback = 300000.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float WeaponMinSpreadAim = 2.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float WeaponMinSpreadWalk = 4.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float WeaponMaxSpread = 15.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float WeaponSpreadPerShot = 4.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter|Weapon")
	float WeaponSpreadRecoveryRate = 1.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Shooter|Weapon")
	float CurrentSpread = 0.0f;

	UPROPERTY(Replicated)
	FVector socketTransform = FVector();

	void BeginPlay() override;

	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// InitiateShoot needs to be called before Shoot
	FShoot InitiateShoot();

	// apply damage if the shoot (the raycast from InstantiateShoot) was effective
	bool Shoot(FShoot shoot);

	// play visual effects
	UFUNCTION()//BlueprintCallable, Category = "Character|Shooter|Weapon")
	void PlayShotFX(FShoot shoot);

	bool NeedsReloading() const;
	void Reload();

	void GetAmmo(int Count);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
