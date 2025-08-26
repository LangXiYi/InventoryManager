// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "InvSys_CommonType.h"
#include "InvSys_InventoryWidget.generated.h"

class UInvSys_BaseInventoryObject;
class UInvSys_InventoryComponent;

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UInvSys_InventoryModule_Display;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory Widget")
	virtual void InitInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject);

	virtual void RemoveFromParent() override;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnRefreshWidget();

public:
	/**
	 * Getter or Setter
	 */

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent.Get();
	}

	template<class T>
	FORCEINLINE T* GetInventoryComponent() const
	{
		return (T*)GetInventoryComponent();		
	}

	FGameplayTag GetSlotTag() const;

	UInvSys_BaseInventoryObject* GetInventoryObject();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Container Grid Layout", meta = (ExposeOnSpawn))
	TObjectPtr<UInvSys_BaseInventoryObject> InventoryObject;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Widget")
	TWeakObjectPtr<UInvSys_InventoryComponent> InventoryComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Container Grid Layout")
	FGameplayTag InventoryTag;

private:
	bool bInitInventoryWidget = false;
};
