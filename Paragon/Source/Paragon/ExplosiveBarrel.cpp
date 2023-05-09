// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosiveBarrel.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "ImpactPoint.h"

// Sets default values
AExplosiveBarrel::AExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Default root"));
	SetRootComponent(DefaultRoot);

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel static mesh"));
	BarrelMesh->SetupAttachment(DefaultRoot);

	ImpactCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Impact collision sphere"));
	ImpactCollisionSphere->SetSphereRadius(50.f);
	ImpactCollisionSphere->SetupAttachment(DefaultRoot);
}

// Called when the game starts or when spawned
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
	ImpactCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AExplosiveBarrel::OnImpactCollisionSphereBeginOverlap);
}

void AExplosiveBarrel::OnImpactCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AImpactPoint* WeaponImpactPoint = Cast<AImpactPoint>(OtherActor);
	if (WeaponImpactPoint)
	{
		ImpactToExplode -= WeaponImpactPoint->GetImpactAmount();
		if (ImpactToExplode <= 0)
		{
			PlayExplosionSound();
			PlayExplosionParticles();
			Destroy();
		}
		
		WeaponImpactPoint->Destroy();
	}
}

void AExplosiveBarrel::PlayExplosionSound()
{
	if (!ExplosionSound)
		return;

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
}

void AExplosiveBarrel::PlayExplosionParticles()
{
	if (!ExplosionParticles)
		return;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticles, GetActorLocation());
}

// Called every frame
void AExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

