// ©David Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GenericStructs.generated.h"


/**
 * 
 */

USTRUCT(BlueprintType)
struct TOWERSHOOTER_API FResourceType: public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		FString ResourceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		int32 Count;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		bool bIsSpendable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		TSoftObjectPtr<UTexture2D> Thumbnail;

};


class TOWERSHOOTER_API GenericStructs
{
public:
	GenericStructs();
	~GenericStructs();
};