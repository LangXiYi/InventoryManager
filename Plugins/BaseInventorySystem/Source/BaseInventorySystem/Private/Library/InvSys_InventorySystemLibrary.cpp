// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/InvSys_InventorySystemLibrary.h"

#include "Components/InvSys_InventoryComponent.h"
#include "Components/InvSys_InventoryControllerComponent.h"
#include "Interface/InvSys_InventoryInterface.h"
#include "Kismet/GameplayStatics.h"

UInvSys_InventoryComponent* UInvSys_InventorySystemLibrary::FindInventoryComponent(AActor* InActor)
{
	if (InActor == nullptr)
	{
		return nullptr;
	}

	IInvSys_InventoryInterface* InvInterface= Cast<IInvSys_InventoryInterface>(InActor);
	if (InvInterface)
	{
		return InvInterface->GetInventoryComponent();
	}
	
	return InActor->FindComponentByClass<UInvSys_InventoryComponent>();
}

UInvSys_InventoryControllerComponent* UInvSys_InventorySystemLibrary::FindInvControllerComponent(const UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
		if (PlayerController)
		{
			return PlayerController->FindComponentByClass<UInvSys_InventoryControllerComponent>();
		}
	}
	return nullptr;
}