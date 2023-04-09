// Fill out your copyright notice in the Description page of Project Settings.


#include "ImpactPoint.h"

// Sets default values
AImpactPoint::AImpactPoint() :
	ImpactAmount(0.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AImpactPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AImpactPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

