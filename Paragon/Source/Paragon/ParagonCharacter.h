// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ParagonCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_ReloadingState UMETA(DisplayName = "ReloadingState"),

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	/// Scene component to use its location for interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	/// Number of items interping to/at this location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount; 
};

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
	void FireTimerCallback();

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

	/**
	 * Initialize the ammo map with ammo values
	 */
	void InitializeAmmoMap();

	/**
	 * Check to make sure if weapon has ammo
	 */
	bool IsWeaponHasAmmo();

	/**
	 * Fire weapon functions
	 */
	void FirePlaySound();

	void FireSendBullet();

	void FirePlayAnim();

	void FirePlayFeedback();

	/**
	 * Bound to the R key and gamepad Face Button Left
	 */
	void ReloadButtonPressed();

	/**
	 * Handle reloading of the weapon
	 */
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	//Checks to see if we have ammo of the equipped EquippedWeapon's ammo type
	bool IsHaveAmmo();

	void CrouchButtonPressed();

	virtual void Jump() override;

	/// Interps capsule half height when crouching/standing
	void InterpCapsuleHalfHeight(float DeltaTime);

	void ResetPickUpSoundTimer();

	void ResetEquipSoundTimer();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * Called from animation blueprint with GrabClip notify
	 */
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	/**
	* Called from animation blueprint with ReleaseClip notify
	*/
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	/**
	 * Called from animation blueprint with HideClip notify
	 */
	UFUNCTION(BlueprintCallable)
	void HideClip();

	/**
	 * Called from animation blueprint with ShowClip notify
	 */
	UFUNCTION(BlueprintCallable)
	void ShowClip();

	void StartAiming();

	void StopAiming();

	void PickupAmmo(class AAmmo* Ammo);

	void InitializeInterpLocations();

	void SpawnImpactPoint(const FVector& ImpactPlace);
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

	//Distance ouward from the camera for the item interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	//Distance upward from the camera for the item interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	//Map to keep of ammo of the different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	//Starting amount of 9mm ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	//Starting amount of 9mm ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	//Combat state can only Fire or Reload only when Onoccupied
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CurrentCombatState;

	//Reload animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	//Transform of the clip when we first grabbed the clip during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	//Scene component to attach to the character's hand during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	//True when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;

	/// Regular movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	/// Crouch movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	/// Current half height of the capsule
	float CurrentCapsuleHalfHeight;

	/// Half height of the capsule when not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandindCapsuleHalfHeight;

	/// Half height of the capsule when crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	/// Ground friction while not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	/// Ground friction while crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	/// Used for knowing whether is aiming button pressed
	bool bIsAimingButtonPressed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpPlace1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpPlace2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpPlace3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpPlace4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpPlace5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpPlace6;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpPlaceWeapon;

	/// Array of interp location structs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickUpSoundTimer;

	FTimerHandle EquipSoundTimer;

	bool bIsShouldPlayPickUpSound;

	bool bIsShouldPlayEquipSound;

	/// Time to wait before we can play another equip sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

	/// Time to wait before we can play another pick up sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickUpSoundResetTime;

	//Class of Impact Point to spawn it in the place of bullet impact
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Environment, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AImpactPoint> WeaponImpactPointClass;
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

	UFUNCTION(BlueprintCallable)
	void UseBeginEvent();

	UFUNCTION(BlueprintCallable)
	void UseEndEvent();

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	//Adds/subtracts to/from OverlappedItemCount and updates bShouldTraceForItems
	void ChangeOverlappedItemCount(int8 Amount);

	FVector GetCameraInterpLocation();

	void GetPickupItem(AItemBase* Item);

	FORCEINLINE ECombatState GetCombatState() const { return CurrentCombatState; }

	FORCEINLINE bool GetIsCrouching() const { return bIsCrouching; }

	FInterpLocation GetInterpLocation(int32 Index);

	/// Returns the index in interplocations array with the lowest item count
	/// E.g. Search for the less busy place to interp to
	int32 GetInterpLocationIndex();

	void IncrementInterpLocationCount(int32 Index, int32 Amount);

	FORCEINLINE bool GetIsShouldPlayPickUpSound() const { return bIsShouldPlayPickUpSound; }

	FORCEINLINE bool GetIsShouldPlayEquipSound() const { return bIsShouldPlayEquipSound; }

	void StartPickUpSoundTimer();

	void StartEquipSoundTimer();

};
