// ©David Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "GenericStructs.generated.h"


/**
 * 
 */

USTRUCT(BlueprintType)
struct FResourceType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FString ResourceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 Count;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	class UTexture2D* Thumbnail;

};


class TOWERSHOOTER_API GenericStructs
{
public:
	GenericStructs();
	~GenericStructs();
};