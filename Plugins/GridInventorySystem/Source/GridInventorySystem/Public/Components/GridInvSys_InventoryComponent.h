// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/InvSys_InventoryComponent.h"
#include "GridInvSys_InventoryComponent.generated.h"



/***
 * GridItem ----> ContainerGrid ----> InventoryContainer ----> InventoryComponent
 * Item --------> Grid--------------> Container -------------> Inventory
 * 库存组件可以由单个或多个容器组成，每个容器包含单个或多个网格系统，网格系统则是由网格单元组成
 */
UCLASS(ClassGroup=(InventorySystem), meta=(BlueprintSpawnableComponent), Blueprintable)
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventoryComponent : public UInvSys_InventoryComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGridInvSys_InventoryComponent();

	UFUNCTION(BlueprintCallable)
	virtual void AddInventoryItemToGridContainer(FGridInvSys_InventoryItem GridContainerItem);

	virtual void AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem, FName TargetSlotName) override;
	
	UUserWidget* GetInventoryLayoutWidget() const
	{
		return InventoryLayoutWidget;
	}
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnInitInventoryObjects_Implementation(APlayerController* NewPlayerController) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** 库存组件的展示菜单 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TSubclassOf<UUserWidget> InventoryLayoutWidgetClass;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	TObjectPtr<UUserWidget> InventoryLayoutWidget;
};
