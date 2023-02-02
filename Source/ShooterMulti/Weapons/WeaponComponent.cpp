#include "WeaponComponent.h"
#include "BeamLight.h"
#include "DamageTypeRifle.h"
#include "../Characters/ShooterCharacter.h"
#include "../Controllers/ShooterController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	LightPool.BeginPlay(GetWorld(), 4u);

	AmmoCount = MaxAmmo;
	CurrentSpread = 0.f;
	if (AmmoCount > WeaponMagazineSize)
	{
		AmmoCount -= WeaponMagazineSize - LoadedAmmo;
		LoadedAmmo = WeaponMagazineSize;
	}
	else
	{
		LoadedAmmo = AmmoCount;
		AmmoCount = 0;
	}
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	ShootTimer += DeltaTime;

	//update spread
	AShooterCharacter* Character = static_cast<AShooterCharacter*>(GetOwner());
	float MinSpread = (Character->GetState() == EShooterCharacterState::Aim) ? WeaponMinSpreadAim : WeaponMinSpreadWalk;
	CurrentSpread = FMath::Max(MinSpread, CurrentSpread - WeaponSpreadRecoveryRate * DeltaTime);
}

FShoot UWeaponComponent::InitiateShoot()
{
	return FShoot{ GetOwner(), ShootRaycast(GetOwner()), GetWeaponData() };
}

bool UWeaponComponent::Shoot(FShoot shoot)
{
	if (ShootTimer < FireRate)
		return false;

	ShootTimer = 0.f;

	if (LoadedAmmo <= 0)
		return false;

	--LoadedAmmo;

	// FIRE ON CLIENT

	// play visual effects
	PlayShotFX(shoot);

	// apply damage or knock back on potentially hit object
	ApplyHitOnServer(shoot);

	return true;
}

void UWeaponComponent::ApplyHit(FShoot shoot)
{
	FLaserWeaponData WeaponData = shoot.WeaponData;
	FHitResult HitResult = shoot.RaycastInfo.HitResult;
	FVector LookDirection = shoot.RaycastInfo.LookDirection;

	//make damages
	FPointDamageEvent DamageEvent = FPointDamageEvent(WeaponData.Damages, HitResult, LookDirection, UDamageTypeRifle::StaticClass());
	HitResult.Actor->TakeDamage(WeaponData.Damages, DamageEvent, nullptr, shoot.Shooter);

	//push hit actors (physics)
	TArray<UActorComponent*> SkeletalMeshComponents;
	HitResult.Actor->GetComponents(USkeletalMeshComponent::StaticClass(), SkeletalMeshComponents);
	for (auto ActorComponent : SkeletalMeshComponents)
	{
		USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ActorComponent);
		if (SkeletalMeshComponent->IsSimulatingPhysics())
			SkeletalMeshComponent->AddForceAtLocation(LookDirection * WeaponData.Knockback, HitResult.ImpactPoint, HitResult.BoneName);
	}
	TArray<UActorComponent*> StaticMeshComponents;
	HitResult.Actor->GetComponents(UStaticMeshComponent::StaticClass(), StaticMeshComponents);
	for (auto ActorComponent : StaticMeshComponents)
	{
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(ActorComponent);
		if (StaticMeshComponent->IsSimulatingPhysics())
			StaticMeshComponent->AddForceAtLocation(LookDirection * WeaponData.Knockback, HitResult.ImpactPoint, HitResult.BoneName);
	}
}

void UWeaponComponent::ApplyHitOnServer_Implementation(FShoot shoot)
{
	if (ShootTimer < FireRate)
		return;

	ShootTimer = 0.f;

	if (LoadedAmmo <= 0)
		return;

	--LoadedAmmo;

	if (shoot.RaycastInfo.bHitCharacter)
		ApplyHit(shoot);
}

FLaserWeaponData UWeaponComponent::GetWeaponData() const
{
	FLaserWeaponData WeaponData;
	WeaponData.MuzzleTransform = GetSocketTransform("MuzzleFlashSocket");
	WeaponData.LookTransform = Cast<AShooterCharacter>(GetOwner())->GetCameraComponent()->GetCameraHandle()->GetComponentTransform();
	WeaponData.Damages = WeaponDamage;
	WeaponData.Knockback = WeaponKnockback;
	WeaponData.Spread = CurrentSpread;

	return WeaponData;
}

FRaycastInfo UWeaponComponent::ShootRaycast(AActor* Causer)
{
	FHitResult HitResult;

	FLaserWeaponData WeaponData = GetWeaponData();
	FVector LookLocation = WeaponData.LookTransform.GetLocation();
	FVector LookDirection = WeaponData.LookTransform.GetRotation().GetForwardVector();

	//apply spread
	if (WeaponData.Spread > 0.f)
		LookDirection = UKismetMathLibrary::RandomUnitVectorInConeInRadians(LookDirection,
			FMath::DegreesToRadians(WeaponData.Spread * .5f));

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Causer);
	CollisionParams.bTraceComplex = true;
	CollisionParams.bReturnPhysicalMaterial = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult,
		LookLocation,
		LookLocation + LookDirection * WeaponData.MaxDistance,
		ECC_Visibility, CollisionParams);

	// no actors hit
	if (!bHit)
	{
		HitResult.ImpactPoint = LookLocation + LookDirection * WeaponData.MaxDistance;
		HitResult.Distance = WeaponData.MaxDistance;
	}

	return FRaycastInfo{
		bHit,
		LookLocation,
		LookDirection,
		HitResult,
		Cast<ACharacter>(HitResult.Actor) != nullptr
	};
}

void UWeaponComponent::PlayShotFX(FShoot shoot)
{
	FLaserWeaponData WeaponData = shoot.WeaponData;
	FRaycastInfo info = shoot.RaycastInfo;
	FHitResult HitResult = info.HitResult;

	if (info.bHit)
	{
		//make impact decal
		MakeImpactDecal(HitResult, ImpactDecalMat, .9f * ImpactDecalSize, 1.1f * ImpactDecalSize);

		//create impact particles
		MakeImpactParticles(ImpactParticle, HitResult, .66f);
	}

	//make the beam visuals
	MakeLaserBeam(WeaponData.MuzzleTransform.GetLocation(), HitResult.ImpactPoint, BeamParticle, BeamIntensity, FLinearColor(1.f, 0.857892f, 0.036923f), BeamIntensityCurve);

	//play the shot sound
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShotSound, WeaponData.MuzzleTransform.GetLocation());

	//make muzzle smoke
	UGameplayStatics::SpawnEmitterAttached(MuzzleSmokeParticle, this, FName("MuzzleFlashSocket"));

	//apply shake
	auto PlayerController = Cast<AShooterController>(Cast<AShooterCharacter>(GetOwner())->GetController());
	if (PlayerController && ShootShake)
		PlayerController->ClientPlayCameraShake(ShootShake);

	//add spread
	CurrentSpread = FMath::Min(WeaponMaxSpread, CurrentSpread + WeaponSpreadPerShot);

	//play sound if gun empty
	if (LoadedAmmo == 0)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShotEmptySound, GetOwner()->GetActorLocation());
}

bool UWeaponComponent::NeedsReloading() const
{
	return LoadedAmmo <= 0;
}

void UWeaponComponent::Reload()
{
	if (AmmoCount > WeaponMagazineSize)
	{
		AmmoCount -= WeaponMagazineSize - LoadedAmmo;
		LoadedAmmo = WeaponMagazineSize;
	}
	else
	{
		LoadedAmmo = AmmoCount;
		AmmoCount = 0;
	}
}

void UWeaponComponent::GetAmmo(int Count)
{
	AmmoCount = FMath::Min(AmmoCount + Count, MaxAmmo);
}

// Weapon Utiliy
void UWeaponComponent::MakeImpactDecal(const FHitResult& FromHit,
	UMaterialInterface* ImpactDecalMaterial,
	float ImpactDecalSizeMin,
	float ImpactDecalSizeMax)
{
	auto StaticMeshComponent = FromHit.Actor->FindComponentByClass<UStaticMeshComponent>();
	if (StaticMeshComponent)
	{
		FVector DecalPos = FromHit.ImpactPoint;
		FRotator DecalRot = (FromHit.ImpactNormal.Rotation().Quaternion() * FRotator(0.f, 0.f, FMath::RandRange(-180.f, 180.f)).Quaternion()).Rotator();
		float RandomSize = FMath::RandRange(ImpactDecalSizeMin, ImpactDecalSizeMax);
		FVector DecalSize = FVector(RandomSize, RandomSize, RandomSize);

		if (StaticMeshComponent->Mobility == EComponentMobility::Static)
		{
			UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ImpactDecalMaterial, DecalSize,
				DecalPos, DecalRot, 0.f);
			if (DecalComponent)
				DecalComponent->FadeScreenSize = 11.f;
		}
		else
		{
			UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAttached(ImpactDecalMaterial, DecalSize, StaticMeshComponent,
				NAME_None, DecalPos, DecalRot, EAttachLocation::KeepWorldPosition, 0.f);
			if (DecalComponent)
				DecalComponent->FadeScreenSize = 11.f;
		}
	}
}

void UWeaponComponent::MakeLaserBeam(FVector Start,
	FVector End,
	UParticleSystem* BeamParticles,
	float InBeamIntensity,
	FLinearColor Color,
	UCurveFloat* InBeamIntensityCurve)
{
	FTransform ParticleTransform;
	ParticleTransform.SetLocation(Start);
	ParticleTransform.SetRotation((End - Start).Rotation().Quaternion());

	//create a beam particle
	SpawnEmitterAtLocation(BeamParticles, ParticleTransform, Start, End);

	LightPool.Spawn(0.8f)->Initialize(Start, End, Color, 0.8f, InBeamIntensity, InBeamIntensityCurve);
}

void UWeaponComponent::MakeImpactParticles(UParticleSystem* ImpactParticles, const FHitResult& FromHit, float Scale)
{
	FTransform HitTransform;
	HitTransform.SetLocation(FromHit.ImpactPoint);
	HitTransform.SetRotation(FromHit.Normal.Rotation().Quaternion());
	HitTransform.SetScale3D(FVector(Scale, Scale, Scale));

	SpawnEmitterAtLocation(ImpactParticles, HitTransform);
}

void UWeaponComponent::SpawnEmitterAtLocation(UParticleSystem* EmitterTemplate,
	const FTransform& SpawnTransform,
	const FVector& Source,
	const FVector& Target)
{
	if (EmitterTemplate)
	{
		UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			EmitterTemplate,
			SpawnTransform,
			true,
			EPSCPoolMethod::AutoRelease);

		if (Source != FVector::ZeroVector && Target != FVector::ZeroVector && ParticleSystemComponent != NULL)
		{
			ParticleSystemComponent->SetBeamSourcePoint(0, Source, 0);
			ParticleSystemComponent->SetBeamTargetPoint(0, Target, 0);
		}
	}
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponComponent, AmmoCount);
	DOREPLIFETIME(UWeaponComponent, LoadedAmmo);
	DOREPLIFETIME(UWeaponComponent, socketTransform);
}