// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridLayoutWidget.generated.h"

class UGridInvSys_ContainerGridWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridLayoutWidget : public UGridInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ConstructContainerGrid(FName SlotName);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Container Grid Layout")
	UGridInvSys_ContainerGridWidget* FindContainerGrid(FName GridID);

	void GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	

private:
	void Private_GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray, UWidget* Parent);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Container Grid Layout")
	TArray<UGridInvSys_ContainerGridWidget*> ContainerGridWidgets;

	UPROPERTY()
	TMap<FName, UGridInvSys_ContainerGridWidget*> ContainerGridMap;
};
