#include "ShooterCharacter.h"
#include "../Animations/ShooterCharacterAnim.h"
#include "../GameFramework/PlayerGI.h"
#include "../LD/EnemySpawnerButton.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AShooterCharacter::AShooterCharacter()
{
	DisappearingDelay = 1.5f;

	// Animation is set in ShooterCharacter_BP to fix build.
	//// Set Animations
	//ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimContainer(TEXT("AnimBlueprint'/Game/Blueprints/Animations/ShooterAnim_BP.ShooterAnim_BP'"));

	//if (AnimContainer.Succeeded())
	//	GetMesh()->SetAnimInstanceClass(AnimContainer.Object->GeneratedClass);

	// Create Weapon
	Weapon = CreateDefaultSubobject<UWeaponComponent>("Rifle");

	ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshContainer(TEXT("SkeletalMesh'/Game/Weapons/Rifle.Rifle'"));
	if (MeshContainer.Succeeded())
		Weapon->SetSkeletalMesh(MeshContainer.Object);

	Weapon->SetRelativeLocation(FVector(1.0f, 4.0f, -2.0f));
	Weapon->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Weapon->SetupAttachment(GetMesh(), "hand_r");

	// Create Camera
	Camera = CreateDefaultSubobject<UPlayerCameraComponent>("PlayerCamera");
	Camera->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
}

EShooterCharacterState AShooterCharacter::GetState() const
{
	return State;
}

float AShooterCharacter::GetStateSpeed() const
{
	switch (State)
	{
	case EShooterCharacterState::IdleRun :
		return RunSpeed;
	case EShooterCharacterState::Aim :
		return AimWalkSpeed;
	case EShooterCharacterState::Sprint :
		return SprintSpeed;
	case EShooterCharacterState::Reload :
		return ReloadWalkSpeed;
	case EShooterCharacterState::Jump :
		return 0.f;
	case EShooterCharacterState::Falling :
		return 0.f;
	case EShooterCharacterState::Punch :
		return 0.f;
	case EShooterCharacterState::Dead :
		return 0.f;
	case EShooterCharacterState::PushButton :
		return 0.f;
	default:
		break;
	}

	return 0.f;
}

void AShooterCharacter::SetState(EShooterCharacterState InState)
{
	PrevState = State;
	State = InState;
}

UWeaponComponent* AShooterCharacter::GetWeaponComponent()
{
	return Weapon;
}

UPlayerCameraComponent* AShooterCharacter::GetCameraComponent()
{
	return Camera;
}

void AShooterCharacter::InitPlayer()
{
	if (GetLocalRole() != ROLE_AutonomousProxy)
		return;

	const FPlayerInfo& PlayerInfo = static_cast<UPlayerGI*>(GetGameInstance())->GetUserInfo();

	InitTeamColorOnServer(static_cast<ETeam>(PlayerInfo.TeamNum));
}

void AShooterCharacter::OnRep_Team()
{
	OnTeamSwitch.Broadcast();
}

void AShooterCharacter::InitTeamColorOnServer_Implementation(ETeam InTeam)
{
	SetTeam(InTeam);
}

void AShooterCharacter::Invincibility(float Duration)
{
	Health = 100000;
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [this]() { Health = MaxHealth; }, Duration, false);

	InvincibilityFX(Duration);
}

void AShooterCharacter::BeginPlay()
{
	OnTeamSwitch.AddLambda([this]() { RefreshTeamHUD(Team); });

	Super::BeginPlay();

	RunSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (GetLocalRole() == ENetRole::ROLE_Authority)
		Invincibility(Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->InvincibilityTime);
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
		return;

	GetCharacterMovement()->MaxWalkSpeed = GetStateSpeed() * movementMultiplyer;


	if (GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
	{
		UpdateShooterRotation();
		UpdateShooterWeapon();
	}

	Camera->ShakeCamera(uint8(State), GetLastMovementInputVector().Size());;
}

void AShooterCharacter::UpdateShooterRotation()
{
	// Anim aim offsets
	FRotator LookRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());
	AimPitch = UKismetMathLibrary::ClampAngle(LookRotation.Pitch, -90.f, 90.f);
	AimYaw = UKismetMathLibrary::ClampAngle(LookRotation.Yaw, -90.f, 90.f);

	AimOffsetsOnServer(AimPitch, AimYaw);
}

void AShooterCharacter::StartSprint()
{
	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Reload)
		AbortReload();
	else if (State == EShooterCharacterState::Aim)
		EndAim(true);

	if (State != EShooterCharacterState::IdleRun && State != EShooterCharacterState::Jump)
		return;

	if (State == EShooterCharacterState::Jump)
		PrevState = EShooterCharacterState::Sprint;
	else
		SetState(EShooterCharacterState::Sprint);

	//GetCharacterMovement()->MaxWalkSpeed = SprintSpeed * movementMultiplyer;

	StartSprintOnServer();
}

void AShooterCharacter::StartSprintOnServer_Implementation()
{
	StartSprint();
}

void AShooterCharacter::EndSprint()
{
	if (State != EShooterCharacterState::Sprint && State != EShooterCharacterState::Jump)
		return;

	if (State == EShooterCharacterState::Jump)
		PrevState = EShooterCharacterState::IdleRun;
	else
		SetState(EShooterCharacterState::IdleRun);

	//GetCharacterMovement()->MaxWalkSpeed = RunSpeed * movementMultiplyer;

	EndSprintOnServer();
}

void AShooterCharacter::EndSprintOnServer_Implementation()
{
	EndSprint();
}

void AShooterCharacter::StartJump()
{
	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Aim)
		EndAim(true);
	else if (State == EShooterCharacterState::Reload)
		AbortReload();
	else if (State == EShooterCharacterState::Sprint)
		EndSprint();

	if (CanJump() && (State == EShooterCharacterState::IdleRun || State == EShooterCharacterState::Sprint))
	{
		SetState(EShooterCharacterState::Jump);
		Jump();
	}
}

void AShooterCharacter::EndJump()
{
	if (State != EShooterCharacterState::Jump && State != EShooterCharacterState::Falling)
		return;

	SetState(EShooterCharacterState::IdleRun);
	StopJumping();
}

void AShooterCharacter::StartAim(bool bStart)
{
	if (State != EShooterCharacterState::IdleRun)
		return;

	SetState(EShooterCharacterState::Aim);

	//GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed * movementMultiplyer;

	if (bStart)
	{
		Camera->SwitchToAimCamera();
		StartAimOnServer();
	}
}

void AShooterCharacter::StartAimOnServer_Implementation()
{
	StartAim(false);
}

void AShooterCharacter::EndAim(bool bEnd)
{
	if (State != EShooterCharacterState::Aim)
		return;

	SetState(PrevState);

	//GetCharacterMovement()->MaxWalkSpeed = RunSpeed * movementMultiplyer;

	if (bEnd)
	{
		Camera->SwitchToWalkCamera();

		EndAimOnServer();
	}
}

void AShooterCharacter::EndAimOnServer_Implementation()
{
	EndAim(false);
}

void AShooterCharacter::StartShoot()
{
	if (State == EShooterCharacterState::IdleRun || State == EShooterCharacterState::Aim)
		bIsShooting = true;
}

void AShooterCharacter::EndShoot()
{
	bIsShooting = false;
}

void AShooterCharacter::StartReload()
{
	if (Weapon && Weapon->AmmoCount > 0 && Weapon->WeaponMagazineSize > Weapon->LoadedAmmo)
	{
		if (State == EShooterCharacterState::Aim)
			EndAim(true);
		else if (bIsShooting)
			bIsShooting = false;

		if (State != EShooterCharacterState::IdleRun)
			return;

		SetState(EShooterCharacterState::Reload);

		//GetCharacterMovement()->MaxWalkSpeed = ReloadWalkSpeed * movementMultiplyer * movementMultiplyer;
	}
}

void AShooterCharacter::AbortReload()
{
	if (State != EShooterCharacterState::Reload)
		return;

	SetState(EShooterCharacterState::IdleRun);

	//GetCharacterMovement()->MaxWalkSpeed = RunSpeed * movementMultiplyer;
}

void AShooterCharacter::Falling()
{
	Super::Falling();

	if (State == EShooterCharacterState::Jump)
		return;

	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Aim)
		EndAimOnServer();
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	SetState(EShooterCharacterState::Falling);
}

void AShooterCharacter::PushButtonOnServer_Implementation()
{
	PushButtonMulticast();
}

void AShooterCharacter::InflictPushButton()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, TSubclassOf<AEnemySpawnerButton>());

	if (OverlappingActors.Num() > 0)
	{
		AEnemySpawnerButton* Button = Cast<AEnemySpawnerButton>(OverlappingActors[0]);

		if (Button)
			Button->Activate(Team);
	}
}

void AShooterCharacter::PlayPushButtonAnim()
{
	Cast<UShooterCharacterAnim>(GetMesh()->GetAnimInstance())->PlayPushButtonMontage();
}

void AShooterCharacter::PlayPunchAnim()
{
	Cast<UShooterCharacterAnim>(GetMesh()->GetAnimInstance())->PlayPunchMontage();
}

void AShooterCharacter::StartDisappearMulticast_Implementation()
{
	Super::StartDisappearMulticast_Implementation();

	FTimerHandle Handle1;
	GetWorld()->GetTimerManager().SetTimer(Handle1, [this]() { Weapon->SetVisibility(false, true); }, 3.5f, false);

	if (Controller)
	{
		APlayerController* PlayerControler = Cast<APlayerController>(Controller);
		PlayerControler->DisableInput(PlayerControler);

		FTimerHandle Handle2;
		GetWorld()->GetTimerManager().SetTimer(Handle2, [PlayerControler]() { PlayerControler->EnableInput(PlayerControler); }, 5.0f, false);
	}
}

void AShooterCharacter::FinishDisappear()
{
	if (GetLocalRole() != ENetRole::ROLE_Authority)
		return;

	APlayerController* PlayerController = Cast<APlayerController>(Controller);

	Super::FinishDisappear();

	Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->RespawnOnServer(PlayerController);
}

void AShooterCharacter::AimOffsetsOnServer_Implementation(float Pitch, float Yaw)
{
	AimPitch = Pitch;
	AimYaw = Yaw;
}

void AShooterCharacter::UpdateShooterWeapon()
{
	if (bIsShooting && !Shoot() && Weapon->NeedsReloading())
		StartReload();

	if (State == EShooterCharacterState::Reload)
		ReloadOnServer();
}

void AShooterCharacter::ReloadOnServer_Implementation()
{
	ReloadMulticast();
}

void AShooterCharacter::ReloadMulticast_Implementation()
{
	if (GetLocalRole() != ENetRole::ROLE_AutonomousProxy)
		StartReload();
}

void AShooterCharacter::EndReloadOnServer_Implementation()
{
	EndReloadMulticast();
}

void AShooterCharacter::EndReloadMulticast_Implementation()
{
	if (State != EShooterCharacterState::Reload)
		return;

	SetState(EShooterCharacterState::IdleRun);

	//GetCharacterMovement()->MaxWalkSpeed = RunSpeed * movementMultiplyer;

	if (Weapon)
		Weapon->Reload();
}

void AShooterCharacter::PunchOnServer_Implementation()
{
	PunchMulticast();
}

void AShooterCharacter::PunchMulticast_Implementation()
{
	if (bIsShooting)
		bIsShooting = false;
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	if (State != EShooterCharacterState::IdleRun)
		return;

	SetState(EShooterCharacterState::Punch);
	PlayPunchAnim();
}

void AShooterCharacter::PushButtonMulticast_Implementation()
{
	if (bIsShooting)
		bIsShooting = false;
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	if (State != EShooterCharacterState::IdleRun)
		return;

	SetState(EShooterCharacterState::PushButton);
	PlayPushButtonAnim();
}

bool AShooterCharacter::Shoot()
{
	FShoot shoot = Weapon->InitiateShoot();

	if (Weapon->Shoot(shoot))
	{
		PlayShotFXOnServer(shoot);
		return true;
	}
	else
	{
		return false;
	}
}

void AShooterCharacter::PlayShotFXOnServer_Implementation(FShoot shoot)
{
	PlayShotFXMulticast(shoot);
}

void AShooterCharacter::PlayShotFXMulticast_Implementation(FShoot shoot)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
		Weapon->PlayShotFX(shoot);
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, State);
	DOREPLIFETIME(AShooterCharacter, Weapon);
	DOREPLIFETIME(AShooterCharacter, AimPitch);
	DOREPLIFETIME(AShooterCharacter, AimYaw);
	DOREPLIFETIME(AShooterCharacter, movementMultiplyer);
}

void AShooterCharacter::ResetEffectPickupOnServer_Implementation()
{
	movementMultiplyer = 1.f;
}