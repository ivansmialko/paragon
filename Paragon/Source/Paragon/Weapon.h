// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class PARAGON_API AWeapon : public AItemBase
{
	GENERATED_BODY()
public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

protected:

	/**
	 * Timer callback function
	 */
	void StopFalling();

private:

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bIsFalling;

	//Ammo count for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 AmmoAmount;

public:

	/**
	 * Adds an impulse to the weapon
	 */
	void ThrowWeapon();

	FORCEINLINE int32 GetAmmoAmount() const { return AmmoAmount; }

	/**
	 * Called from ParagonCharacter when firing weapon
	 */
	void DecrementAmmoAmount();
};
