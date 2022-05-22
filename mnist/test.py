import torch
import random
import utils
#from model.Lenet5 import LENET5
from model.VGG16 import VGG16
load_checkpoints="checkpoints/VGG16"
test_path = "test_data"

display_points = 5000


def test():
    # Usage GPU
    seed = random.randint(1, 10000)
    torch.manual_seed(seed)
    torch.cuda.manual_seed(seed)
    device = "cuda" if torch.cuda.is_available() else "cpu"

    # data load
    test_data = utils.grayscale_image_load(test_path,test=True)
    datasets = utils.MNIST_DATASETS(test_data=test_data,Istest=True)
    test_loader = datasets.configure_datasets()

    # model load


    #model = LENET5()
    model = VGG16()
    model.load_state_dict(torch.load(load_checkpoints))
    model.eval().to(device)

    with torch.no_grad():
        total_correct = 0
        for num, i in enumerate(test_loader):
            test_y = i[1].long().to(device)
            test_x = i[0].float().to(device)

            pred_y = model(test_x)
            correct_ = utils.calculate_accuracy(pred_y,test_y,1,display=True,epoch=num+1,display_points = display_points)
            total_correct += correct_
        avg_accuracy = total_correct / (len(test_loader)*1)

    print("test_accuracy : {:>.2}".format(avg_accuracy))


if __name__ == "__main__":
    test()
