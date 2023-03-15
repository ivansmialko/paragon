// Fill out your copyright notice in the Description page of Project Settings.


#include "ParagonPlayerController.h"
#include "Blueprint/UserWidget.h"

AParagonPlayerController::AParagonPlayerController()
{

}

void AParagonPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!MainHudClass)
		return;

	MainHudInstance = CreateWidget<UUserWidget>(this, MainHudClass);
	if (!MainHudInstance)
		return;

	MainHudInstance->AddToViewport();
	MainHudInstance->SetVisibility(ESlateVisibility::Visible);
}
