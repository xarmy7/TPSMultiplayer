
#include "AmmoPickup.h"
#include "../Characters/ShooterCharacter.h"

void AAmmoPickup::NotifyActorBeginOverlap(AActor * OtherActor)
{
	AShooterCharacter* Player = Cast<AShooterCharacter>(OtherActor);
	UWeaponComponent* Weapon = Player ? Player->GetWeaponComponent() : nullptr;

	if (!Weapon || Weapon->AmmoCount >= Weapon->MaxAmmo)
		return;

	Weapon->GetAmmo(Capacity);

	// After because of Destroy
	Super::NotifyActorBeginOverlap(OtherActor);
}