// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvSys_InventoryWidget.generated.h"

class UInvSys_InventoryComponent;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp);

	UInvSys_InventoryComponent* GetInventoryComponent() const;

	template<class T>
	T* GetInventoryComponent() const
	{
		return Cast<T>(InventoryComponent);		
	}
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Widget")
	TObjectPtr<UInvSys_InventoryComponent> InventoryComponent;
};
