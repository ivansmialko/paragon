// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_PickUp UMETA(DisplayName = "PickUp"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Weapon UMETA(DisplayName = "Weapon"),
	EIT_Ammo UMETA(DisplayName = "Ammo"),

	EIT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightWidgetColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkWidgetColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;

	//Used to switch colors of outline
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;
};

UCLASS()
class PARAGON_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * Called when overlapping sphere
	 */
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/**
	 * Called when overlapping sphere is over
	 */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	/**
	 * Sets the ActiveStars array of bools based on rarity
	 */
	void UpdateActiveStars();

	/**
	 * Sets properties of an Item's components based on state
	 * @param State Input state, properties will be based on
	 */
	virtual void SetItemProperties(EItemState State);

	/**
	 * Called when interpolation timer is finished
	 */
	void FinishFlying();

	/**
	 * Handles item iterpolation when in the EquipInterping state
	 * @param DeltaTime Delta time from Tick()
	 */
	void ItemInterp(float DeltaTime);

	/**
	 * Play sound of picking the item
	 */
	void PlayPickupSound(bool bForcePlaySound = false);

	/**
	 * Get interp location based on the item type
	 */
	FVector GetInterpLocation();

	virtual void OnConstruction(const FTransform& Transform) override;

	void ResetPulseTimer();

	void UpdatePulse();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	* Play sound of equipping the item
	*/
	void PlayEquipSound(bool bIsForce = false);
private:
	//Skeletal mesh for the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* ItemMesh;

	//Linetrace collides with this box to show HUD widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	//Pop-up widget for when the player looks at the item
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupInfoWidget;

	//Enables item-tracing when overlapped
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	//The name which appears on the Pickup Widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	//Item count (ammo, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	//Item rarity determines number of stars in pickup widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	//Stars states for item rarity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	//Current state of the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	//The curve asset to use for the items Z location when interping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;

	//Starting location when item position interpolation to camera begins
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	//Target interp location in front of the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	//True when interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bIsInterping;

	//Plays when we start interping
	FTimerHandle ItemInterpTimer;

	//Pointer to the character
	class AParagonCharacter* PlayerCharacter;

	//Duration of the interping timer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float InterpTimerDuration;

	//X and Y for the ItemBase while interping in the EquipInterping state
	float ItemInterpX;
	float ItemInterpY;

	//Initial Yaw offset between the camera and the interping item
	float InterpInitalYawOffset;

	//Curve used to scale the item when flying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

	//Sound played when item is picked up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;

	//Sound player when item is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USoundCue* EquipSound;

	/// Enum for the current item type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;

	/// Index of the interp location this item is interping to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocationIndex;

	/// Index for the material we'd like to change at runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	int32 LastMaterialIndex;

	/// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	/// Material instance used with the Dynamic Material Instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	bool bIsCanChangeCustomDepth;

	/// Curve to drive the dynamic material parameters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveVector* PulseCurve;

	FTimerHandle PulseTimer;

	/// Time for the pulse timer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	/// Parameter of glow material instance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	/// Parameter of glow material instance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;

	/// Parameter of glow material instance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;

	/// Curve to drive the dynamic material parameters when interpint
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* PulseInterpCurve;

	/// An icon for the item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;

	/// An ammo icon for the item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconAmmo;

	/// Slot in the inventory array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	/// True when the caracter's inventory is full
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	bool bIsCharacterInventoryFull;

	/// Item rarity data table
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDataTable;

	/// Color in the glow material
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	/// Light color in the pickup widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor LightWidgetColor;

	/// Dark color in the pickup widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkWidgetColor;

	/// Number of stars in the pickup widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	/// Background icon for the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;

public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupInfoWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	/// Don't forget to call UpdateItemProperties() to update properties corresponding to the state!
	FORCEINLINE void SetItemState(EItemState State) { ItemState = State; }

	void UpdateItemProperties();

	FORCEINLINE USkeletalMeshComponent* GetItemMesh() { return ItemMesh; }
	//Called from the AParagonCharacter to begin item position interpolating to camera
	void StartItemFlying(AParagonCharacter* Character, bool bForcePlaySound = false);

	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; };
	void SetPickupSound(class USoundCue* val) { PickupSound = val; }

	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	void SetEquipSound(USoundCue* val) { EquipSound = val; }

	FORCEINLINE int32 GetItemCount() const { return ItemCount; }

	FORCEINLINE void SetCharacterRef(AParagonCharacter* RefCharacter) { PlayerCharacter = RefCharacter; }

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();

	virtual void InitializeCustomDepth();

	void EnableGlowMaterial();
	void DisableGlowMaterial();

	int32 GetSlotIndex() const { return SlotIndex; }
	void SetSlotIndex(int32 Index) { SlotIndex = Index; }

	FORCEINLINE bool GetIsCharacterInventoryFull() const { return bIsCharacterInventoryFull; }
	FORCEINLINE void SetIsCharacterInventoryFull(bool val) { bIsCharacterInventoryFull = val; }

	FORCEINLINE void SetItemName(FString val) { ItemName = val; }

	FORCEINLINE UTexture2D* GetIconItem() const { return IconItem; }
	FORCEINLINE void SetIconItem(UTexture2D* val) { IconItem = val; }

	FORCEINLINE UTexture2D* GetIconAmmo() const { return IconAmmo; }
	FORCEINLINE void SetIconAmmo(UTexture2D* val) { IconAmmo = val; }

	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* val) { MaterialInstance = val; }

	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }
	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* val) { DynamicMaterialInstance = val; }

	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }
	FORCEINLINE void SetGlowColor(FLinearColor val) { GlowColor = val; }

	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }
	FORCEINLINE void SetMaterialIndex(int32 val) { MaterialIndex = val; }

	FORCEINLINE int32 GetLastMaterialIndex() const { return LastMaterialIndex; }
	FORCEINLINE void SetLastMaterialIndex(int32 val) { LastMaterialIndex = val; }
};
