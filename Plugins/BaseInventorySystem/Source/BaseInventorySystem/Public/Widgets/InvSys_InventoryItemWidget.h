// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "InvSys_InventoryItemWidget.generated.h"

struct FInvSys_DragItemInstanceMessage;
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
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	virtual void NativeOnDragItem(bool bIsDraggingItem);

	UFUNCTION(BlueprintImplementableEvent)
	void OnEnableDragItemInstance();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDisableDragItemInstance();

protected:
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Inventory Item", meta = (ExposeOnSpawn))
	TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;

private:
	bool bIsEnableDragItem = true;

	FTimerHandle ServerTimeoutHandle;
};
