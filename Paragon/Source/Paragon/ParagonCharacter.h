// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ParagonCharacter.generated.h"

UCLASS()
class PARAGON_API AParagonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AParagonCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called for forwards/backwards input
	void MoveForward(float in_value);

	//Called for left/right input
	void MoveRight(float in_value);

	/**
	 * Called via input to turn at a given rate
	 * @param in_rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float in_rate);

	/**
	 * Called via input to look up/down at a given rate
	 * @param in_rate	This is a normalized rate, i.e. 1.0 means 100% of desired look up/down rate
	 */
	void LookUpAtRate(float in_rate);

	/**
	 * Rotate controller based on mouse X movement
	 * @param Value Input axis value
	 */
	void Turn(float Value);

	/**
	 * Rotate controller based on mouse Y movement
	 * @param Value Input axis value
	 */
	void LookUp(float Value);

	/**
	 * Called when the "Fire" button is pressed
	 */
	void FireWeapon();


	/**
	 * Calculates impact point of a bullet, based on position of the muzzle and direction of the crosshair
	 * @param MuzzleSocketLocation Location of gun's muzzle
	 * @param OutBeamLocation Reference to a variable, used as output
	 */
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	/**
	 * Set bIsAminig to true with button pressed
	 */
	void SetAimingButtonPressed();

	/**
	* Set bIsAminig to true with button released
	*/
	void SetAimingButtonReleased();

	/**
	 * Smoothly zoom in / zoom out camera when aiming
	 * @param DeltaTime Delta time from Tick() function
	 */
	void ZoomCameraInterp(float DeltaTime);

	/**
	 * Set turn/look up rates based on aiming or not
	 */
	void UpdateLookRates();

	/**
	 * Calculate the spread of the crosshairs
	 * @param DeltaTime Delta time from Tick() function
	 */
	void UpdateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	/**
	 * Sets bIsFireButtonPressed to true, to begin firing
	 */ 
	void FireButtonPressed();

	/**
	 * Sets bIsFireButtonPressed to stop firing
	 */ 
	void FireButtonReleased();

	/**
	 * Start timer for automatic firing
	 */
	void StartFireTimer();

	/**
	 * Allows to fire next bullet, called when fire-rate delay is ended
	 */
	UFUNCTION()
	void AutoFireReset();

	/**
	 * Line trace for items under the crosshairs
	 * @param OutHitResult Reference to variable with the result of line trace hit
	 */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	/**
	 * Tracing for items to set their widgets visibility to true if trace is successful
	 */
	void TraceForItems();

	/**
	 * Spawns default weapon and equips it
	 */
	class AWeapon* SpawnDefaultWeapon();

	/**
	 * Takes a weapon and attaches it to the mesh
	 * @param WeaponToEquip Self explanatory
	 */
	void EquipWeapon(AWeapon* WeaponToEquip);

	/**
	 * Detach weapon and let it fall to the ground
	 */
	void DropWeapon();

	void SelectButtonPressed();

	void SelectButtonReleased();

	/**
	 * Drops currently equipped Weapon and equips TraceHitItem
	 * @param WeaponToSwap Other weapon you need to swap current weapon with
	 */
	void SwapWeapon(AWeapon* WeaponToSwap);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:

	// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// Camera that follows the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;	

	//Base turn rate in deg/sec. Other scaling  may affect final turn rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	//Base look up/down rate, in deg/sec. Other scaling may affect final turn rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	//Controller turn sensitivity while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	//Controller lookup sensitivity while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	//Controller turn sensitivity while aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	//Controller look up rate while aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	//Mouse turn sensitivity while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	//Mouse look up sensitivity while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	//Mouse turn sensitivity while aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	//Mouse look up sensitivity while aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	//Randomized gunshot sound cue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	//Flash spawned at barrel socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	//Particles spawned upon bullet impact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	//Smoke trail for bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	//Montage for firing recoil
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* FireRecoilMontage;

	//Effect to call vibration on gamepad
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UForceFeedbackEffect* FeedbackFire;

	//True when aiming
	bool bIsAiming;

	//Default camera field of view value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatCamera, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	//Zoomed-in field of view value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatCamera, meta = (AllowPrivateAccess = "true"))
	float CameraZoomFOV;	

	//Current field of view at this frame
	float CameraCurrentFOV;

	//Interp speed for zooming when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatCamera, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatCamera, meta = (AllowPrivateAccess = "true"))
	FVector CameraOffset;

	//Determines the spread of the crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	//Velocity component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityScaleFactor;

	//In-air component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirScaleFactor;

	//Aim component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimScaleFactor;

	//Shooting component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	float ShootTimerDuration;

	bool bIsFiringBullet;

	FTimerHandle CrosshairShootTimer;

	//Fire button pressed
	bool bIsFireButtonPressed;

	//True when we can fire, false when waiting for timer
	bool bIsShouldFireAtThisFrame;

	//Rate of automatic gun fire
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AutomaticFireRate;

	//Sets a timer between gunshots
	FTimerHandle AutoFireTimer;

	//True if we should trace every frame for items
	bool bIsShouldTraceForItems;

	//Number of overlapped AItemBase items
	int8 OverlappedItemCount;

	//Item that player is currently focused on
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItemBase* TraceHitLastFrame;

	//Currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	//Set this in blueprints for the default Weapon class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	//The item, trace is currently hit on in TraceForItems (could be null)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItemBase* TraceHitItem;
public:

	//Return CameraBoom subobject
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; };
	
	//Returns FollowCamera subobject
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; };

	//Returns bIsAiming
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }

	//Get spread to render crosshairs accordingly
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const { return CrosshairSpreadMultiplier; };

	UFUNCTION(BlueprintCallable)
	void FireBeginEvent();

	UFUNCTION(BlueprintCallable)
	void FireEndEvent();

	UFUNCTION(BlueprintCallable)
	void FireSingleEvent();

	UFUNCTION(BlueprintCallable)
	void JumpEvent();

	UFUNCTION(BlueprintCallable)
	void AimEvent();

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	//Adds/subtracts to/from OverlappedItemCount and updates bShouldTraceForItems
	void ChangeOverlappedItemCount(int8 Amount);
};
