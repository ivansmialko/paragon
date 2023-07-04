// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExplosive::AExplosive():
	ExplosionDamage(100.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
	SetRootComponent(ExplosiveMesh);

	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	DamageSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* DamageInstigator)
{
	if (!ExplosionSound)
		return;

	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());

	if (!ExplosionParticles)
		return;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticles, HitResult.Location);

	//Apply explosive damage
	TArray<AActor*> OverlapingActors;
	GetOverlappingActors(OverlapingActors, ACharacter::StaticClass());
	for (const auto& Actor : OverlapingActors)
	{
		UGameplayStatics::ApplyDamage(Actor, ExplosionDamage, DamageInstigator, Shooter, UDamageType::StaticClass());
	}

	Destroy();
}

