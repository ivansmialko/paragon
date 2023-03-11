// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"	
#include "ParagonCharacter.h"

// Sets default values
AItemBase::AItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupInfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickupInfoWidget->SetupAttachment(RootComponent);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	
	//Hide pick up widget by default
	if (!PickupInfoWidget)
		return;

	PickupInfoWidget->SetVisibility(false);

	//Setup overlap for AreaSphere
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemBase::OnSphereBeginOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItemBase::OnSphereEndOverlap);
}

void AItemBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;

	AParagonCharacter* ParagonCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!ParagonCharacter)
		return;

	ParagonCharacter->ChangeOverlappedItemCount(1);

}

void AItemBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent,int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;

	AParagonCharacter* ParagonCharacter = Cast<AParagonCharacter>(OtherActor);
	if (!ParagonCharacter)
		return;

	ParagonCharacter->ChangeOverlappedItemCount(-1);
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

