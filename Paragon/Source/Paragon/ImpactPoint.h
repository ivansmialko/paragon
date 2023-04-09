// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ImpactPoint.generated.h"

UCLASS()
class PARAGON_API AImpactPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AImpactPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	int32 ImpactAmount;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE void SetImpactAmount(int32 InImpactAmount) {
		ImpactAmount = InImpactAmount;
	}

	FORCEINLINE int32 GetImpactAmount() const { return ImpactAmount; }

};
