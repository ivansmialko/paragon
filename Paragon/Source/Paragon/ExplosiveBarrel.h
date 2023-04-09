// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosiveBarrel.generated.h"

UCLASS()
class PARAGON_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/// Called when any object colliding with Collision Sphere
	UFUNCTION()
		void OnImpactCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComponent,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);

	/// Plays an explosion sound
	void PlayExplosionSound();

	/// Plays an explosion particles
	void PlayExplosionParticles();

private:
	
	/// Default root for an blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DefaultSceneRoot;

	/// Static mesh for barrel model
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel properties", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BarrelMesh;

	/// Sphere that will collide with ImpactPoint object, meant to detect any impact that can detonate the barrel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* ImpactCollisionSphere;

	/// Explosion sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* ExplosionSound;

	/// Explosion particles
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel properties", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ExplosionParticles;

	/// Amount of impact required to explode a barrel
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel properties", meta = (AllowPrivateAccess = "true"))
	float ImpactToExplode;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
