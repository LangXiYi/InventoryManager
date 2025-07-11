// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryWidget.h"
#include "InvSys_InventoryItemWidget.generated.h"

class UInvSys_InventoryItemInstance;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	UInvSys_InventoryComponent* GetPlayerInventoryComponent() const;
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
public:
	void SetItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);
	
	template<class T>
	T* GetItemInstance() const
	{
		return (T*)ItemInstance.Get();
	}

	template<class T>
	T* GetPlayerInventoryComponent() const
	{
		return (T*)GetPlayerInventoryComponent();
	}

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item", meta = (ExposeOnSpawn))
	TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;
};
