// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "Engine/SkeletalMeshSocket.h"

#include "DrawDebugHelpers.h"

#include "EnemyController.h"
#include "ParagonCharacter.h"

// Sets default values
AEnemy::AEnemy() :
	CurrentHealth(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(4.f),
	bIsCanHitReact(true),
	HitReactDelayMin(0.5f),
	HitReactDelayMax(3.f),
	HitNumberDestroyTime(1.5f),
	bIsStunned(false),
	StunChance(0.5f),
	AttackLFast(TEXT("Attack_L_Fast")),
	AttackRFast(TEXT("Attack_R_Fast")),
	AttackL(TEXT("Attack_L")),
	AttackR(TEXT("Attack_R")),
	BaseDamage(20.f),
	LeftWeaponSocketName(TEXT("FX_Trail_L_01")),
	RightWeaponSocketName(TEXT("FX_Trail_R_01")),
	bIsCanAttack(true),
	AttackWaitTime(1.f),
	bIsDying(false),
	DestroyTime(3.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRange"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	if (!GetMesh())
		return;

	// Construct left and right weapon collision boxed
	LeftWeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponCollisionBox"));
	LeftWeaponCollisionBox->SetupAttachment(GetMesh(), FName("left_weapon_bone"));

	RightWeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponCollisionBox"));
	RightWeaponCollisionBox->SetupAttachment(GetMesh(), FName("right_weapon_bone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	//Ignore the camera for Mesh and Capsule
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	CurrentHealth = MaxHealth;

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlapBegin_AgroSphere);

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlapBegin_CombatRangeSphere);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnOverlapEnd_CombatRangeSphere);

	// Get AI Controller
	EnemyController = Cast<AEnemyController>(GetController());
	if (!EnemyController)
		return;

	if (!EnemyController->GetBlackboardComponent())
		return;

	if (!EnemyController->GetBehaviorTreeComponent())
		return;

	FVector WorldPatrolPoint{ UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint1) };
	EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint1"), WorldPatrolPoint);
	DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);

	WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
	EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint);
	DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);

	EnemyController->RunBehaviorTree(BehaviorTree);

	LeftWeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlapBegin_LeftWeaponCollisionBox);
	RightWeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlapBegin_RightWeaponCollisionBox);

	LeftWeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	RightWeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	EnemyController->GetBlackboardComponent()->SetValueAsBool("IsCanAttack", bIsCanAttack);
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarDisappearTimer);
	GetWorldTimerManager().SetTimer(HealthBarDisappearTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
	if (bIsDying)
		return;

	bIsDying = true;

	HideHealthBar();

	for (const auto& HitNumberPair : HitNumberWidgets)
	{
		HitNumberPair.Key->RemoveFromParent();
	}

	HitNumberWidgets.Empty();
	//Destroy();
	PlayDeathMontage("Death_A");
	EnemyController->GetBehaviorTreeComponent()->StopTree(EBTStopMode::Forced);
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate /*= 1.0f*/)
{
	if (!bIsCanHitReact)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	AnimInstance->Montage_Play(HitMontage, PlayRate);
	AnimInstance->Montage_JumpToSection(Section, HitMontage);

	bIsCanHitReact = false;
	const float HitReactDelay{ FMath::FRandRange(HitReactDelayMin, HitReactDelayMax) };

	GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactDelay);
}

void AEnemy::PlayDeathMontage(FName Section, float PlayRate /*= 1.0f*/)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	AnimInstance->Montage_Play(DeathMontage, PlayRate);
	AnimInstance->Montage_JumpToSection(Section, DeathMontage);

	if (!EnemyController)
		return;

	EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("IsDead"), true);
	EnemyController->StopMovement();
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate /*= 1.0f*/)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	if (!AttackMontage)
		return;

	AnimInstance->Montage_Play(AttackMontage, PlayRate);
	AnimInstance->Montage_JumpToSection(Section, AttackMontage);

	bIsCanAttack = false;
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetIsCanAttack, AttackWaitTime);

	if (!EnemyController)
		return;

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsCanAttack"), bIsCanAttack);
}

void AEnemy::ResetHitReactTimer()
{
	bIsCanHitReact = true;
}

void AEnemy::StoreHitNumberWidget(UUserWidget* HitNumberWidget, FVector WidgetLocation)
{
	HitNumberWidgets.Add(HitNumberWidget, WidgetLocation);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumberWidget);
	GetWorldTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumberWidgets.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
	for (auto& HitPair : HitNumberWidgets)
	{
		FVector2D ScreenPosition;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), HitPair.Value, ScreenPosition);

		HitPair.Key->SetPositionInViewport(ScreenPosition);
	}
}

void AEnemy::OnOverlapBegin_AgroSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;

	AParagonCharacter* PlayerCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!PlayerCharacter)
		return;

	EnemyController->GetBlackboardComponent()->SetValueAsObject("ChaseTarget", PlayerCharacter);
}

void AEnemy::OnOverlapBegin_CombatRangeSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Begin overlap"));

	if (!OtherActor)
		return;

	AParagonCharacter* ParagonCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!ParagonCharacter)
		return;

	if (!EnemyController)
		return;

	if (!EnemyController->GetBlackboardComponent())
		return;

	bInAttackRange = true;
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsInAttackRange"), bInAttackRange);
}

void AEnemy::OnOverlapEnd_CombatRangeSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("OverlapEnd"));

	if (!OtherActor)
		return;

	AParagonCharacter* ParagonCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!ParagonCharacter)
		return;

	if (!EnemyController)
		return;

	if (!EnemyController->GetBlackboardComponent())
		return;

	bInAttackRange = false;
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsInAttackRange"), bInAttackRange);
}

void AEnemy::OnOverlapBegin_LeftWeaponCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AttackActor(OtherActor);

	AParagonCharacter* PlayerCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!PlayerCharacter)
		return;

	SpawnBloodParticles(LeftWeaponSocketName, PlayerCharacter);
	StunCharacter(PlayerCharacter);
}

void AEnemy::OnOverlapBegin_RightWeaponCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AttackActor(OtherActor);

	AParagonCharacter* PlayerCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!PlayerCharacter)
		return;

	SpawnBloodParticles(RightWeaponSocketName, PlayerCharacter);
	StunCharacter(PlayerCharacter);
}

FName AEnemy::GetAttackSectionName()
{
	const int32 Section{ FMath::RandRange(1, 4) };

	switch (Section)
	{
	case 1:
	{
		return AttackLFast;
	}
	case 2:
	{
		return AttackRFast;
	}
	case 3:
	{
		return AttackL;
	}
	case 4:
	{
		return AttackR;
	}
	default:
	{
		return AttackR;
	}
	}
}

void AEnemy::ActivateLeftWeapon()
{
	if (!LeftWeaponCollisionBox)
		return;

	LeftWeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeActivateLeftWeapon()
{
	if (!LeftWeaponCollisionBox)
		return;

	LeftWeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	if (!RightWeaponCollisionBox)
		return;

	RightWeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeActivateRightWeapon()
{
	if (!RightWeaponCollisionBox)
		return;

	RightWeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AEnemy::SpawnBloodParticles(FName WeaponSocketName, AParagonCharacter* PlayerCharacter)
{
	const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName(WeaponSocketName);
	if (TipSocket && PlayerCharacter->GetBloodParticles())
	{
		const FTransform SocketTransform{ TipSocket->GetSocketTransform(GetMesh()) };
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerCharacter->GetBloodParticles(), SocketTransform);
	}
}

void AEnemy::StunCharacter(AParagonCharacter* Victim)
{
	if (!Victim)	
		return;

	const float CurrentStunChance = FMath::FRandRange(0.f, 1.f);
	if (CurrentStunChance < Victim->GetStunChance())
	{
		Victim->Stun();
	}
}

void AEnemy::ResetIsCanAttack()
{
	bIsCanAttack = true;

	if (!EnemyController)
		return;

	EnemyController->GetBlackboardComponent()->SetValueAsBool("IsCanAttack", bIsCanAttack);
}

void AEnemy::AttackActor(AActor* TargetActor)
{
	if (!TargetActor)
		return;

	UGameplayStatics::ApplyDamage(TargetActor, BaseDamage, EnemyController, this, UDamageType::StaticClass());
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AEnemy::DestroyEnemy, DestroyTime);

	Destroy();
}

void AEnemy::DestroyEnemy()
{
	Destroy();
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if ((CurrentHealth - DamageAmount) < 0.0f)
	{
		CurrentHealth = 0.f;
		Die();
	}
	else
	{
		CurrentHealth -= DamageAmount;
	}

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("ChaseTarget"), DamageCauser);
	}

	return DamageAmount;
}

void AEnemy::SetStunned(bool InIsStunned)
{
	if (!EnemyController)
		return;

	if (!EnemyController->GetBlackboardComponent())
		return;

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsStunned"), InIsStunned);
	bIsStunned = InIsStunned;
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
	if (!ImpactSound)
		return;

	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());

	if (!ImpactParticles)
		return;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);

	if (bIsDying)
		return;

	ShowHealthBar();

	//Determinte whether bullet hit stuns
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		// Stun the enemy
		PlayHitMontage(FName("HitReact_Front"));
		SetStunned(true);
	}
}

