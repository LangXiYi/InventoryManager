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
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);
	
	template<class T>
	T* GetItemInstance() const
	{
		return (T*)ItemInstance.Get();
	}

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	virtual void NativeDestruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item", meta = (ExposeOnSpawn))
	TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;

private:
	bool bIsWaitingServerResponse = false;

	FTimerHandle ServerTimeoutHandle;
};
