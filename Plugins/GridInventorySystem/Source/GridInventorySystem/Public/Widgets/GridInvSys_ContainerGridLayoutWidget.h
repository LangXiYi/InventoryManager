// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridLayoutWidget.generated.h"

struct FGridInvSys_ItemPosition;

class UInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridItemWidget;
class UGridInvSys_ContainerGridWidget;

UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridLayoutWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	virtual void InitInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject) override;

	UFUNCTION(BlueprintCallable, Category = "Grid Grid Layout")
	void AddItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Grid Grid Layout")
	void AddItemInstanceTo(UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPosition);

	UFUNCTION(BlueprintCallable, Category = "Grid Grid Layout")
	void RemoveAllItemInstance();

	UFUNCTION(BlueprintCallable, Category = "Grid Grid Layout")
	void RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Container Grid Layout")
	void RemoveItemInstanceFrom(UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPosition);

	UFUNCTION(BlueprintPure, Category = "Container Grid Layout", meta = (UnsafeDuringActorConstruction))
	UGridInvSys_ContainerGridWidget* FindContainerGrid(int32 GridID);

	UFUNCTION(BlueprintPure, Category = "Container Grid Layout", meta = (UnsafeDuringActorConstruction))
	UGridInvSys_ContainerGridItemWidget* FindGridItemWidgetByPos(const FGridInvSys_ItemPosition& ItemPosition) const;

	UFUNCTION(BlueprintPure, Category = "Container Grid Layout", meta = (UnsafeDuringActorConstruction))
	UGridInvSys_ContainerGridItemWidget* FindGridItemWidget(const UInvSys_InventoryItemInstance* InItemInstance) const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	TArray<UGridInvSys_ContainerGridWidget*> GetContainerGridWidgets() const
	{
		return ContainerGridWidgets;
	}

	void GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray) const;

private:
	void Debug_PrintContainerAllItems();
	void Private_GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray, UWidget* Parent) const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Container Grid Layout")
	TArray<UGridInvSys_ContainerGridWidget*> ContainerGridWidgets;

	UPROPERTY()
	TMap<FName, UGridInvSys_ContainerGridWidget*> ContainerGridMap;

private:
	FGameplayMessageListenerHandle OnAddItemInstanceHandle;
	FGameplayMessageListenerHandle OnRemoveItemInstanceHandle;
	FGameplayMessageListenerHandle OnItemPositionChangedHandle;
};
