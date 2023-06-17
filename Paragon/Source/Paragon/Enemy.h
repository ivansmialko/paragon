// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Enemy.generated.h"

UCLASS()
class PARAGON_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/// <summary>
	/// Function to be called in the blueprint and in the code, when health bar needs to be showed
	/// </summary>
	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();
	void ShowHealthBar_Implementation();

	/// <summary>
	/// Function to be called in the blueprint when health bar needs to be hidden
	/// </summary>
	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	/// <summary>
	/// Destroy the enemy
	/// </summary>
	void Die();

	/// <summary>
	/// Play animation of being hit by a player
	/// </summary>
	/// <param name="Section">Name of animation inside of AnimationMontage</param>
	/// <param name="PlayRate">Speed of animation</param>
	void PlayHitMontage(FName Section, float PlayRate = 1.0f);

	/// <summary>
	/// Play animation of enemy death
	/// </summary>
	/// <param name="Section">Name of animation inside of AnimationMontage</param>
	/// <param name="PlayRate">Speed of animation</param>
	void PlayDeathMontage(FName Section, float PlayRate = 1.0f);

	/// <summary>
	/// Play attack animation of the enemy to player
	/// </summary>
	/// <param name="Section">Name of animation inside of AnimationMontage</param>
	/// <param name="PlayRate">Speed of animation</param>
	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate = 1.0f);
	
	/// <summary>
	/// Reset timer with timeout to the next play of "Hit" animation
	/// </summary>
	void ResetHitReactTimer();

	/// <summary>
	/// Add UI widget to list, to update it's location and then destroy later
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StoreHitNumberWidget(UUserWidget* HitNumberWidget, FVector WidgetLocation);

	/// <summary>
	/// Destroy a particular UI widget with player-to-enemy damage
	/// </summary>
	UFUNCTION()
	void DestroyHitNumber(UUserWidget* HitNumber);

	/// <summary>
	/// Update position of UI widgets with damage numbers
	/// </summary>
	void UpdateHitNumbers();

	/// <summary>
	/// Called when something overlaps with the agro sphere
	/// </summary>
	UFUNCTION()
	void OnOverlapBegin_AgroSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	/// <summary>
	/// Called when player enters the attack range of enemy
	/// </summary>
	UFUNCTION()
	void OnOverlapBegin_CombatRangeSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);


	/// <summary>
	/// Called when player leaves the attack range of enemy
	/// </summary>
	UFUNCTION()
	void OnOverlapEnd_CombatRangeSphere(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	/// <summary>
	/// Called when enemy's left weapon contacts with player or any object
	/// </summary>
	UFUNCTION()
	void OnOverlapBegin_LeftWeaponCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	/// <summary>
	/// Called when enemy's right weapon contacts with player or any object
	/// </summary>
	UFUNCTION()
	void OnOverlapBegin_RightWeaponCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	/// Activate/deactivate collision for weapon boxes
	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();

	UFUNCTION(BlueprintCallable)
	void DeActivateLeftWeapon();

	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();

	UFUNCTION(BlueprintCallable)
	void DeActivateRightWeapon();

	void AttackActor(AActor* TargetActor);

	/// <summary>
	/// Spawns a particle system at the contact location of enemy's melee and other player
	/// </summary>
	/// <param name="WeaponSocketName">Weapon socket name</param>
	/// <param name="PlayerCharacter">Player character</param>
	void SpawnBloodParticles(FName WeaponSocketName, class AParagonCharacter* PlayerCharacter);

	/// <summary>
	/// Attempt to stunt a character
	/// </summary>
	/// <param name="Victim">Target player</param>
	void StunCharacter(AParagonCharacter* Victim);

	/// <summary>
	/// Resets the "bIsCanAttack" value. Called by AttackWaitTimer
	/// </summary>
	void ResetIsCanAttack();

private:
	/// Particles to spawn when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;
	
	/// Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	/// Current health level of the enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth;

	/// Maximum enemy health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	/// Name of the head bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FString HeadBoneName;

	/// Time to display health bar since shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HealthBarDisplayTime;

	FTimerHandle HealthBarDisappearTimer;

	/// Motage containing hit and death animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	/// Animation montage that contains all attack animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	/// Timer to wait before enemy can play hit animation again
	FTimerHandle HitReactTimer;

	/// True when enemy can play hit animation at this frame
	bool bIsCanHitReact;

	/// Min and max values to delay before enemy can play hit animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float HitReactDelayMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float HitReactDelayMax;

	/// Map to store HitNumber widgets and their locations
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI",  meta = (AllowPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> HitNumberWidgets;

	/// Time before HitNumber is removed from the screen
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	float HitNumberDestroyTime;

	/// Behavior tree for the AI character
	UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	/// Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint1;

	/// Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	/// Reference to controller that possess this enemy
	class AEnemyController* EnemyController;

	/// Overlap sphere, for when the enemy becomes hostile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;

	/// True when playing the "get hit" animation
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	bool bIsStunned;

	/// Chance of being stunned
	/// 0.0 - no stun chance, 1 - 100% stun chance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float StunChance;

	/// True when in attack range; time to attack!
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CombatRangeSphere;

	/// <summary>
	/// The four attack montage section names
	/// </summary>
	FName AttackLFast;
	FName AttackRFast;
	FName AttackL;
	FName AttackR;

	/// Collision volume for the left weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWeaponCollisionBox;

	/// Collision volume for the Right weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* RightWeaponCollisionBox;

	/// Base damage the enemy can deal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	/// Socket for the left enemy's weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocketName;

	/// Socket for the right enemy's weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocketName;

	/// Is can attack at current frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsCanAttack;

	/// Timer to wait for another attack
	FTimerHandle AttackWaitTimer;

	/// Minimum wait time between attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BulletHit_Implementation(FHitResult HitResult) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	FORCEINLINE FString GetHeadBoneName() { return HeadBoneName; }

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumber(int Damage, FVector HitLocation, bool bIsHeadShot);

	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool InIsStunned);
};
