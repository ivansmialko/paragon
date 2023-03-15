// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ParagonPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PARAGON_API AParagonPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AParagonPlayerController();

protected:

	virtual void BeginPlay() override;
	
private:

	//Reference to the class of MainHUD widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GUI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> MainHudClass;

	//Instance of MainHUD widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GUI, meta = (AllowPrivateAccess = "true"))
	UUserWidget* MainHudInstance;
};
