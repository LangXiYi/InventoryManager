// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GridInvSys_CommonType.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GridInvSys_ContainerGridItemWidget.generated.h"

class UInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridWidget;

UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void OnConstructGridItem(UGridInvSys_ContainerGridWidget* InContainerGrid, FIntPoint InPosition);
	
	void AddItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);

	void RemoveItemInstance();

	FORCEINLINE UGridInvSys_ContainerGridWidget* GetContainerGridWidget() const
	{
		return ContainerGridWidget;
	}

	template<class T>
	T* GetItemInfo() const
	{
		return nullptr;
	}

	FORCEINLINE FIntPoint GetPosition() const
	{
		return Position;
	}
	
	FORCEINLINE FIntPoint GetOriginPosition() const
	{
		return OriginGridItemWidget ? OriginGridItemWidget->GetPosition() : GetPosition();
	}

	FORCEINLINE UGridInvSys_ContainerGridItemWidget* GetOriginGridItemWidget() const
	{
		return OriginGridItemWidget;
	}

	FORCEINLINE bool IsOccupied()
	{
		return bIsOccupied;
	}
	
	TArray<UWidget*> GetOccupiedGridItems();

	FIntPoint CalculateRelativePosition(const UGridInvSys_ContainerGridItemWidget* Parent) const;

	UFUNCTION(BlueprintPure)
	int32 GetGridID() const;

	template<class T = UInvSys_InventoryItemInstance>
	T* GetItemInstance() const
	{
		return (T*)ItemInstance.Get();
	}

	FIntPoint GetItemSize() const
	{
		return GridItemSize;
	}

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnAddItemInstance(const UInvSys_InventoryItemInstance* NewItemInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRemoveItemInstance();

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "DraggingDisplayWidget")
	void OnDraggingHovered(bool bIsCanDrop);

	UFUNCTION(BlueprintImplementableEvent, Category = "DraggingDisplayWidget")
	void OnEndDraggingHovered();

private:
	/** 获取Item的大小，该大小会收到方向的影响。 */
	FIntPoint CalculateGridItemSize(UInvSys_InventoryItemInstance* InItemInstance) const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item", meta = (BindWidget))
	TObjectPtr<UNamedSlot> ItemSlot;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item", meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox;
	
	UPROPERTY()
	TObjectPtr<UGridInvSys_ContainerGridWidget> ContainerGridWidget;

	UPROPERTY()
	TObjectPtr<UGridInvSys_ContainerGridItemWidget> OriginGridItemWidget;

	/** Item Position Info */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	FIntPoint Position;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	FIntPoint GridItemSize = FIntPoint(1, 1);

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Inventory Grid Item")
	TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	bool bIsOccupied;
	
	FGameplayMessageListenerHandle DragItemListenerHandle;
};
