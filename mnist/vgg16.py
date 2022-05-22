import torch
import torch.nn as nn

class VGG16(nn.Module):
    def __init__(self):
        super(VGG16,self).__init__()
        self.block_1 = nn.Sequential(
            nn.Conv2d(in_channels=1, out_channels=64, kernel_size= 3, stride=(1, 1), padding=1),
            nn.ReLU(),
            nn.Conv2d(in_channels=64,out_channels=64, kernel_size=(3, 3), stride=(1, 1), padding=1),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=(2, 2), stride=(2, 2))
        )

        self.block_2 = nn.Sequential(
            nn.Conv2d(in_channels=64,out_channels=128, kernel_size=(3, 3), stride=(1, 1), padding=1),
            nn.ReLU(),
            nn.Conv2d(in_channels=128, out_channels=128,kernel_size=(3, 3), stride=(1, 1), padding=1),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=(2, 2),stride=(2, 2))
        )

        self.block_3 = nn.Sequential(
            nn.Conv2d(in_channels=128, out_channels=256, kernel_size=(3, 3), stride=(1, 1), padding=1),
            nn.ReLU(),
            nn.Conv2d(in_channels=256, out_channels=256, kernel_size=(3, 3), stride=(1, 1), padding=1),
            nn.ReLU(),
            nn.Conv2d(in_channels=256,out_channels=256,kernel_size=(3, 3),stride=(1, 1),padding=1),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=(2, 2),stride=(2, 2))
        )

        self.block_4 = nn.Sequential(
            nn.Conv2d(in_channels=256,out_channels=512,kernel_size=(3, 3), stride=(1, 1),padding=1),
            nn.ReLU(),
            nn.Conv2d(in_channels=512,out_channels=512,kernel_size=(3, 3),stride=(1, 1),padding=1),
            nn.ReLU(),
            nn.Conv2d(in_channels=512,out_channels=512,kernel_size=(3, 3),stride=(1, 1),padding=1),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=(2, 2),stride=(2, 2))
        )

        self.block_5 = nn.Sequential(
            nn.Conv2d(in_channels=512,out_channels=512,kernel_size=(3, 3),stride=(1, 1),padding=1),
            nn.ReLU(),
            nn.Conv2d(in_channels=512,out_channels=512,kernel_size=(3, 3),stride=(1, 1),padding=1),
            nn.ReLU(),
            nn.Conv2d(in_channels=512,out_channels=512,kernel_size=(3, 3),stride=(1, 1),padding=1),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=(2, 2),stride=(2, 2))
        )

        height, width = 3, 3
        self.classifier = nn.Sequential(
            nn.Linear(512 * height * width, 4096),
            nn.ReLU(True),
            nn.Dropout(p=0.5),
            nn.Linear(4096, 4096),
            nn.ReLU(True),
            nn.Dropout(p=0.5),
            nn.Linear(4096, 10),
        )

        for m in self.modules():
            if isinstance(m, torch.torch.nn.Conv2d) or isinstance(m, torch.torch.nn.Linear):
                torch.nn.init.kaiming_uniform_(m.weight, mode='fan_in', nonlinearity='relu')
                if m.bias is not None:
                    m.bias.detach().zero_()

        self.avgpool = torch.nn.AdaptiveAvgPool2d((height, width))

    def forward(self, x):

        x = self.block_1(x)
        #print(x.shape)
        x = self.block_2(x)
        x = self.block_3(x)
        x = self.block_4(x)
        x = self.block_5(x)
        x = self.avgpool(x)
        x = x.view(x.size(0), -1)  # flatten

        logits = self.classifier(x)

        return logits
if __name__ == "__main__":
    model = VGG16().to("cuda")
    img_1 = torch.ones([1,1,224,224]).to("cuda")
    pred_ = model(img_1)
    print(model)
    print(pred_.shape)
