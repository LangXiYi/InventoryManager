// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridLayoutWidget.generated.h"

class UGridInvSys_ContainerGridItemWidget;
struct FGridInvSys_ItemPosition;
class UGridInvSys_InventoryContainerInfo;
class UGridInvSys_ContainerGridWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridLayoutWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ConstructContainerGrid(FName SlotName);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Container Grid Layout")
	UGridInvSys_ContainerGridWidget* FindContainerGrid(FName GridID);

	UGridInvSys_ContainerGridItemWidget* FindGridItemWidget(const FGridInvSys_ItemPosition& ItemPosition) const;

protected:
	virtual void NativeOnInitialized() override;

public:
	TArray<UGridInvSys_ContainerGridWidget*> GetContainerGridWidgets() const
	{
		return ContainerGridWidgets;
	}

private:
	void GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray) const;

	void Private_GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray, UWidget* Parent) const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Container Grid Layout")
	TArray<UGridInvSys_ContainerGridWidget*> ContainerGridWidgets;

	UPROPERTY()
	TMap<FName, UGridInvSys_ContainerGridWidget*> ContainerGridMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Grid Layout")
	TObjectPtr<UGridInvSys_InventoryContainerInfo> ContainerInfo;
};
