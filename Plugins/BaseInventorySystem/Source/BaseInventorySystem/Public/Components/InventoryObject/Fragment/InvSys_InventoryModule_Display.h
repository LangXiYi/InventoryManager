// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryModule.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "InvSys_InventoryModule_Display.generated.h"

class UInvSys_InventoryWidget;

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryModule_Display : public UInvSys_InventoryModule
{
	GENERATED_BODY()

public:
	UInvSys_InventoryModule_Display();
	
	virtual void InitInventoryFragment(UObject* PreEditFragment) override;

	virtual void RefreshInventoryFragment() override;

	virtual UInvSys_InventoryWidget* CreateDisplayWidget(APlayerController* PC);

	TSubclassOf<UUserWidget> GetDisplayWidgetClass() const
	{
		return DisplayWidgetClass;
	}

	template<class T>
	T* GetDisplayWidget() const
	{
		if (DisplayWidget && DisplayWidget->IsA<T>())
		{
			return (T*)DisplayWidget;
		}
		return nullptr;
	}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Fragment")
	TSubclassOf<UInvSys_InventoryWidget> DisplayWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment")
	TObjectPtr<UInvSys_InventoryWidget> DisplayWidget = nullptr;
};
